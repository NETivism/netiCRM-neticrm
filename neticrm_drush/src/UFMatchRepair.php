<?php

namespace Drupal\neticrm_drush;

/**
 * Find active CMS accounts whose identity only came from the legacy email match.
 *
 * Before #47831, an account without its own civicrm_uf_match row was matched
 * to a contact by email; when that contact or email already belonged to
 * another uf_id, the row was not saved but the session identity was still
 * granted. Each affected account gets a planned action and a confidence score:
 *
 * - Contact has the account email: primary +40, other email +20.
 * - civicrm_uf_match.uf_name equals the account email: +30.
 * - Current owner UID: deleted +30, blocked +20, active but never logged in +20,
 *   active and logged in -20 (another person may use the contact).
 * - Contact has no account link at all: +30.
 * - Merge/trash log found for the email: +10.
 *
 * Accounts that never logged in have not proven they own the email and are skipped.
 */
class UFMatchRepair {

  /** uf_name row (uf_name = account email) belongs to another UID. */
  const NAME_MATCH = 'name-match';

  /** No uf_name row; the only primary-email contact is linked to another UID. */
  const CONTACT_LINKED = 'contact-linked';

  /** No uf_name row; the only primary-email contact has no account link. */
  const UNLINKED = 'unlinked';

  /** Data does not allow any automatic change. */
  const MANUAL = 'manual';

  /** Default minimum confidence applied by --execute. */
  const MIN_CONFIDENCE = 80;

  /** civicrm_log.data patterns written by merge and trash, in English and zh_TW. */
  const LOG_PATTERNS = [
    '%merge duplicate contacts%',
    '%合併重複的聯絡人%',
    'Delete Contact%',
    '刪除聯絡人%',
  ];

  /**
   * Classify active, logged-in accounts that have no civicrm_uf_match row of their own.
   *
   * @param array $uids Restrict to these Drupal user IDs; empty for all users.
   * @return array Keys: rows (classified accounts), never_logged_in (count of skipped accounts).
   */
  public static function scan(array $uids = []) {
    $domainID = \CRM_Core_Config::domainID();
    $byUID = $byName = $byContact = [];
    $dao = \CRM_Core_DAO::executeQuery('SELECT id, uf_id, uf_name, contact_id FROM civicrm_uf_match WHERE domain_id = %1', [1 => [$domainID, 'Integer']]);
    while ($dao->fetch()) {
      $match = ['id' => (int) $dao->id, 'uf_id' => (int) $dao->uf_id, 'uf_name' => $dao->uf_name, 'contact_id' => (int) $dao->contact_id];
      $byUID[$match['uf_id']] = $match;
      $byName[self::normalize($match['uf_name'])] = $match;
      $byContact[$match['contact_id']] = $match;
    }
    $dao->free();

    // Drupal users may live in another database, so never join across them.
    $users = \Drupal::database()->query('SELECT uid, name, mail, status, login FROM {users_field_data} WHERE uid > 0 AND default_langcode = 1')->fetchAllAssoc('uid', \PDO::FETCH_ASSOC);
    $mailCount = [];
    foreach ($users as $account) {
      $mail = self::normalize($account['mail']);
      if ((int) $account['status'] === 1 && $mail !== '') {
        $mailCount[$mail] = ($mailCount[$mail] ?? 0) + 1;
      }
    }

    $result = ['rows' => [], 'never_logged_in' => 0];
    foreach ($users as $uid => $account) {
      $uid = (int) $uid;
      $mail = self::normalize($account['mail']);
      if (($uids && !in_array($uid, $uids, TRUE)) || isset($byUID[$uid]) || (int) $account['status'] !== 1 || $mail === '') {
        continue;
      }
      if (empty($account['login'])) {
        $result['never_logged_in']++;
        continue;
      }
      $row = self::classify($uid, $mail, $users, $byName, $byContact, $mailCount[$mail]);
      if ($row) {
        $result['rows'][] = $row;
      }
    }
    return $result;
  }

  /**
   * Build the planned action, confidence and conflict for one account.
   *
   * @param int $uid Account without its own association.
   * @param string $mail Normalized account email.
   * @param array $users Drupal users keyed by UID.
   * @param array $byName Associations keyed by normalized uf_name.
   * @param array $byContact Associations keyed by contact ID.
   * @param int $sameMail Number of active accounts using this email.
   * @return array|null Row, or NULL when no contact relates to the email.
   */
  public static function classify($uid, $mail, array $users, array $byName, array $byContact, $sameMail) {
    $match = $byName[$mail] ?? NULL;
    if ($match) {
      $category = self::NAME_MATCH;
      $contactID = $match['contact_id'];
    }
    else {
      $candidates = self::primaryContacts($mail);
      if (!$candidates) {
        return NULL;
      }
      if (count($candidates) > 1) {
        return self::row($uid, self::MANUAL, '', $mail, NULL, 0, 'contacts ' . implode(',', $candidates) . ' share this primary email');
      }
      $contactID = $candidates[0];
      $match = $byContact[$contactID] ?? NULL;
      $category = $match ? self::CONTACT_LINKED : self::UNLINKED;
    }

    $conflicts = [];
    $score = 0;
    $email = self::contactEmail($contactID, $mail);
    if ($email === 'deleted') {
      return self::row($uid, self::MANUAL, $contactID, $mail, $match, 0, "contact $contactID is deleted");
    }
    if ($email === '') {
      return self::row($uid, self::MANUAL, $contactID, $mail, $match, 0, "contact $contactID lacks this email");
    }
    $score += $email === 'primary' ? 40 : 20;
    if ($sameMail > 1) {
      return self::row($uid, self::MANUAL, $contactID, $mail, $match, 0, "$sameMail active accounts share this email");
    }

    if ($category === self::NAME_MATCH) {
      $score += 30;
    }
    if ($match) {
      $owner = $users[$match['uf_id']] ?? NULL;
      $ownerUID = $match['uf_id'];
      if (!$owner) {
        $score += 30;
        $conflicts[] = "owner $ownerUID deleted";
      }
      elseif ((int) $owner['status'] !== 1) {
        $score += 20;
        $conflicts[] = "owner $ownerUID blocked";
      }
      elseif (empty($owner['login'])) {
        $score += 20;
        $conflicts[] = "owner $ownerUID active, never logged in";
      }
      else {
        $score -= 20;
        $conflicts[] = "owner $ownerUID active, last login " . date('Y-m-d', $owner['login']);
      }
    }
    else {
      $score += 30;
    }
    $evidence = self::evidence($mail, [$contactID]);
    if ($evidence !== '') {
      $score += 10;
    }
    $row = self::row($uid, $category, $contactID, $mail, $match, max(0, min(100, $score)), implode('; ', $conflicts));
    $row['evidence'] = $evidence;
    return $row;
  }

  /**
   * @param int $uid Account to repair.
   * @param string $category One of the category constants.
   * @param int|string $contactID Contact the account would be linked to, or ''.
   * @param string $mail Normalized account email.
   * @param array|null $match Existing association to move, if any.
   * @param int $score Confidence, 0-100.
   * @param string $conflict Human-readable conflict, or ''.
   * @return array Row with display fields and the data repair() needs.
   */
  private static function row($uid, $category, $contactID, $mail, $match, $score, $conflict) {
    if ($category === self::MANUAL) {
      $action = 'none';
    }
    elseif ($category === self::UNLINKED) {
      $action = "insert uf_match: uf_id $uid -> contact $contactID";
    }
    else {
      $action = "uf_match #{$match['id']}: uf_id {$match['uf_id']} -> $uid";
      if ($category === self::CONTACT_LINKED) {
        $action .= ', uf_name -> account email';
      }
    }
    return [
      'uid' => $uid,
      'category' => $category,
      'contact_id' => $contactID,
      'action' => $action,
      'confidence' => $score,
      'conflict' => $conflict,
      'evidence' => '',
      'mail' => $mail,
      'match' => $match,
    ];
  }

  /**
   * @param string|null $mail Raw email.
   * @return string Trimmed, lowercased email for comparison.
   */
  public static function normalize($mail) {
    return mb_strtolower(trim((string) $mail), 'UTF-8');
  }

  /**
   * How the contact holds the email, as the legacy email match required.
   *
   * @param int $contactID Contact ID.
   * @param string $mail Normalized email.
   * @return string "primary", "other", "deleted" (contact in trash or gone), or "" (email not on contact).
   */
  public static function contactEmail($contactID, $mail) {
    $dao = \CRM_Core_DAO::executeQuery(
      'SELECT c.is_deleted, MAX(e.is_primary) AS is_primary, COUNT(e.id) AS emails
       FROM civicrm_contact c LEFT JOIN civicrm_email e ON e.contact_id = c.id AND e.email = %2
       WHERE c.id = %1 GROUP BY c.id, c.is_deleted',
      [1 => [$contactID, 'Integer'], 2 => [$mail, 'String']]
    );
    if (!$dao->fetch() || (int) $dao->is_deleted) {
      return 'deleted';
    }
    if (!(int) $dao->emails) {
      return '';
    }
    return (int) $dao->is_primary ? 'primary' : 'other';
  }

  /**
   * @param string $mail Normalized email.
   * @return array IDs of live contacts using the email as primary.
   */
  public static function primaryContacts($mail) {
    $ids = [];
    $dao = \CRM_Core_DAO::executeQuery(
      'SELECT DISTINCT c.id FROM civicrm_email e INNER JOIN civicrm_contact c ON c.id = e.contact_id
       WHERE e.email = %1 AND e.is_primary = 1 AND c.is_deleted = 0 ORDER BY c.id',
      [1 => [$mail, 'String']]
    );
    while ($dao->fetch()) {
      $ids[] = (int) $dao->id;
    }
    $dao->free();
    return $ids;
  }

  /**
   * Summarize merge/trash logs on the contact and on any contact using the email.
   *
   * @param string $mail Normalized email.
   * @param array $contactIDs Contact IDs to include.
   * @return string "cid@date" entries, newest first, or an empty string.
   */
  public static function evidence($mail, array $contactIDs) {
    $params = [1 => [$mail, 'String']];
    $likes = [];
    foreach (self::LOG_PATTERNS as $i => $pattern) {
      $params[$i + 2] = [$pattern, 'String'];
      $likes[] = 'l.data LIKE %' . ($i + 2);
    }
    $contactClause = 'l.entity_id IN (SELECT contact_id FROM civicrm_email WHERE email = %1)';
    if ($contactIDs) {
      $contactClause = "($contactClause OR l.entity_id IN (" . implode(',', array_map('intval', $contactIDs)) . '))';
    }
    $dao = \CRM_Core_DAO::executeQuery(
      "SELECT l.entity_id, l.modified_date FROM civicrm_log l
       WHERE l.entity_table = 'civicrm_contact' AND $contactClause AND (" . implode(' OR ', $likes) . ')
       ORDER BY l.modified_date DESC LIMIT 3',
      $params
    );
    $items = [];
    while ($dao->fetch()) {
      $items[] = $dao->entity_id . '@' . substr($dao->modified_date, 0, 10);
    }
    $dao->free();
    return implode(' ', $items);
  }

  /**
   * Copy civicrm_uf_match to a new table and verify the copy.
   *
   * Must run outside any CRM transaction: CREATE TABLE commits implicitly.
   *
   * @return string Name of the verified backup table.
   * @throws \CRM_Core_Exception When the backup cannot be created or verified.
   */
  public static function backup() {
    if (\CRM_Core_Transaction::isActive()) {
      throw new \CRM_Core_Exception('Cannot back up civicrm_uf_match inside a transaction.');
    }
    $table = 'civicrm_uf_match_bak_' . date('YmdHis');
    if (\CRM_Core_DAO::singleValueQuery('SHOW TABLES LIKE %1', [1 => [$table, 'String']])) {
      throw new \CRM_Core_Exception("Backup table $table already exists.");
    }
    \CRM_Core_DAO::executeQuery("CREATE TABLE `$table` LIKE civicrm_uf_match");
    \CRM_Core_DAO::executeQuery("INSERT INTO `$table` SELECT * FROM civicrm_uf_match");
    $source = self::checksum('civicrm_uf_match');
    $copy = self::checksum($table);
    if ($source !== $copy) {
      throw new \CRM_Core_Exception("Backup table $table does not match civicrm_uf_match ($copy vs $source); nothing was changed.");
    }
    return $table;
  }

  /**
   * Row count and content checksum used to compare a table with its backup.
   *
   * @param string $table Table with the civicrm_uf_match structure.
   * @return string "count:checksum".
   */
  public static function checksum($table) {
    $dao = \CRM_Core_DAO::executeQuery(
      "SELECT COUNT(*) AS total, COALESCE(SUM(CRC32(CONCAT_WS('#', id, domain_id, uf_id, uf_name, contact_id, IFNULL(language, '')))), 0) AS crc FROM `$table`"
    );
    $dao->fetch();
    return $dao->total . ':' . $dao->crc;
  }

  /**
   * Apply the planned action of one row.
   *
   * Re-checks the data under the same UID locks used by login synchronization.
   *
   * @param array $row Row returned by scan().
   * @return string|null Undo SQL for the change, or NULL when skipped.
   */
  public static function repair(array $row) {
    if ($row['category'] === self::MANUAL) {
      return NULL;
    }
    $uid = (int) $row['uid'];
    $contactID = (int) $row['contact_id'];
    $match = $row['match'];
    $domainID = \CRM_Core_Config::domainID();
    $transaction = new \CRM_Core_Transaction();
    try {
      $locked = $transaction->acquireLock('ufmatch.' . $uid);
      if ($locked && $match) {
        $locked = $transaction->acquireLock('ufmatch.' . $match['uf_id']);
      }
      $own = \CRM_Core_DAO::singleValueQuery(
        'SELECT id FROM civicrm_uf_match WHERE domain_id = %1 AND uf_id = %2',
        [1 => [$domainID, 'Integer'], 2 => [$uid, 'Integer']]
      );
      if (!$locked || $own || in_array(self::contactEmail($contactID, $row['mail']), ['', 'deleted'], TRUE)) {
        $transaction->commit();
        return NULL;
      }
      if ($row['category'] === self::UNLINKED) {
        if (\CRM_Core_BAO_UFMatch::hasUFMatchConflict($uid, $row['mail']) || \CRM_Core_DAO::singleValueQuery(
          'SELECT id FROM civicrm_uf_match WHERE domain_id = %1 AND contact_id = %2',
          [1 => [$domainID, 'Integer'], 2 => [$contactID, 'Integer']]
        )) {
          $transaction->commit();
          return NULL;
        }
        \CRM_Core_BAO_UFMatch::saveUFMatch($uid, $row['mail'], $contactID);
        $id = (int) \CRM_Core_DAO::singleValueQuery(
          'SELECT id FROM civicrm_uf_match WHERE domain_id = %1 AND uf_id = %2',
          [1 => [$domainID, 'Integer'], 2 => [$uid, 'Integer']]
        );
        $undo = "DELETE FROM civicrm_uf_match WHERE id = $id;";
        $message = "Linked CMS user $uid by neticrm-ufmatch-repair";
      }
      else {
        // Only move the row if it is still exactly what the scan saw.
        $id = $match['id'];
        $ownerUID = $match['uf_id'];
        $dao = \CRM_Core_DAO::executeQuery(
          'UPDATE civicrm_uf_match SET uf_id = %1, uf_name = %2 WHERE id = %3 AND domain_id = %4 AND uf_id = %5 AND contact_id = %6 AND uf_name = %7',
          [
            1 => [$uid, 'Integer'],
            2 => [$row['mail'], 'String'],
            3 => [$id, 'Integer'],
            4 => [$domainID, 'Integer'],
            5 => [$ownerUID, 'Integer'],
            6 => [$contactID, 'Integer'],
            7 => [$match['uf_name'], 'String'],
          ]
        );
        if ($dao->affectedRows() !== 1) {
          $transaction->commit();
          return NULL;
        }
        $undo = sprintf("UPDATE civicrm_uf_match SET uf_id = %d, uf_name = '%s' WHERE id = %d;", $ownerUID, \CRM_Core_DAO::escapeString($match['uf_name']), $id);
        $message = "Moved CMS user link from UID $ownerUID to $uid by neticrm-ufmatch-repair";
      }
      \CRM_Core_BAO_Log::register($contactID, 'civicrm_uf_match', $id, NULL, $message);
      $transaction->commit();
      return $undo;
    }
    catch (\Throwable $e) {
      $transaction->rollback();
      $transaction->commit();
      throw $e;
    }
  }

}
