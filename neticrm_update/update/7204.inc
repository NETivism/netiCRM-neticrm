<?php
/**
 * @file
 * Update for 7204
 *
 * This file should be civicrm compliant.
 */
function _neticrm_update_update_7204() {
  $replace = [
    'receipt_text|htmlize' => [
      ts('Contributions - Receipt (off-line)'),
      ts('Contributions - Receipt (on-line)'),
      ts('Memberships - Receipt (on-line)'),
    ],
    'confirm_email_text|htmlize' => [
      ts('Events - Registration Confirmation and Receipt (off-line)'),
      ts('Events - Registration Confirmation and Receipt (on-line)'),
    ],
  ];

  foreach ($replace as $string => $templates) {
    $string_new = str_replace('|htmlize', '', $string);
    foreach ($templates as $t) {
      $params = [1 => [$t, 'String']];
      $query = CRM_Core_DAO::executeQuery("SELECT id FROM civicrm_msg_template WHERE msg_title = %1", $params);
      $msgids = [];
      while ($query->fetch()) {
        $msgids[] = $query->id;
      }
      $params = [
        1 => [$string, 'String'],
        2 => [$string_new, 'String'],
      ];
      if (!empty($msgids)) {
        $msgidstring = implode(',', $msgids);
        CRM_Core_DAO::executeQuery("UPDATE civicrm_msg_template SET msg_html = REPLACE(`msg_html`, %1, %2) WHERE id IN($msgidstring)", $params);
      }
    }
  }
  return "Successful update message templates wrong htmlize. (#16961)";
}
