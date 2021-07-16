<?php
function civicrm_statistics_fetch($debug = false){
  $stat = array();
  $sitekey = $_SERVER['HTTP_HOST'];
  $tables = civicrm_statistics_table();
  civicrm_initialize();
  foreach($tables as $table => $t){
    list($table, $sub) = explode('.', $table);
    if($sub){
      switch($t){
        case 'cc.g':
          $rows = civicrm_statistics_query("SELECT gender_id idx, count(id) counting FROM civicrm_contact WHERE gender_id IS NOT NULL AND is_deleted != 1 GROUP BY gender_id");
          break;
        case 'cc.b':
          $rows = civicrm_statistics_query("SELECT year(birth_date) idx, count(id) counting FROM civicrm_contact WHERE birth_date IS NOT NULL AND is_deleted != 1 GROUP BY year(birth_date)");
          break;
        case 'ca.p':
          $rows = civicrm_statistics_query("SELECT a.state_province_id idx, count(a.id) counting FROM civicrm_address a INNER JOIN civicrm_contact c ON c.id = a.contact_id WHERE a.state_province_id IS NOT NULL AND c.is_deleted != 1 group by a.state_province_id");
          break;
        case 'ccb.a':
          $rows = CRM_Core_DAO::singleValueQuery("SELECT SUM(cc.total_amount) FROM civicrm_contribution cc LEFT JOIN civicrm_participant_payment p ON p.contribution_id = cc.id LEFT JOIN civicrm_membership_payment m ON m.contribution_id = cc.id WHERE cc.is_test = 0 AND cc.contribution_status_id = 1 AND p.id IS NULL AND m.id IS NULL AND cc.payment_processor_id IS NOT NULL");
          $rows = (int) $rows;
          break;
        case 'ccb.h':
          $rows = civicrm_statistics_query("SELECT HOUR(cf.receive_date) idx, count(cf.id) counting, round(sum(cf.total_amount)) as summ, round(avg(cf.total_amount)) as avgg FROM (SELECT cc.id, cc.total_amount, cc.receive_date as receive_date FROM civicrm_contribution cc LEFT JOIN civicrm_participant_payment p ON p.contribution_id = cc.id LEFT JOIN civicrm_membership_payment m ON m.contribution_id = cc.id WHERE cc.is_test = 0 AND cc.contribution_status_id = 1 AND p.id IS NULL AND m.id IS NULL AND cc.payment_processor_id IS NOT NULL AND cc.receive_date > '1990-01-01 00:00:00' AND EXTRACT(HOUR_SECOND FROM cc.receive_date) != '000000' AND cc.contribution_recur_id IS NULL UNION SELECT cc.id, cc.total_amount, MIN(cc.receive_date) as receive_date FROM civicrm_contribution cc LEFT JOIN civicrm_participant_payment p ON p.contribution_id = cc.id LEFT JOIN civicrm_membership_payment m ON m.contribution_id = cc.id WHERE cc.is_test = 0 AND cc.contribution_status_id = 1 AND p.id IS NULL AND m.id IS NULL AND cc.payment_processor_id IS NOT NULL AND cc.receive_date > '1990-01-01 00:00:00' AND EXTRACT(HOUR_SECOND FROM cc.receive_date) != '000000' AND contribution_recur_id IS NOT NULL GROUP BY cc.contribution_recur_id ) cf GROUP BY HOUR(cf.receive_date)");
          civicrm_statistics_normalize($rows, 24);
          break;
        case 'ccb.w':
          $rows = civicrm_statistics_query("SELECT WEEKDAY(cf.receive_date) idx, count(cf.id) counting, round(sum(cf.total_amount)) as summ, round(avg(cf.total_amount)) as avgg FROM (SELECT cc.id, cc.total_amount, cc.receive_date as receive_date FROM civicrm_contribution cc LEFT JOIN civicrm_participant_payment p ON p.contribution_id = cc.id LEFT JOIN civicrm_membership_payment m ON m.contribution_id = cc.id WHERE cc.is_test = 0 AND cc.contribution_status_id = 1 AND p.id IS NULL AND m.id IS NULL AND cc.payment_processor_id IS NOT NULL AND cc.receive_date > '1990-01-01 00:00:00' AND EXTRACT(HOUR_SECOND FROM cc.receive_date) != '000000' AND cc.contribution_recur_id IS NULL UNION SELECT cc.id, cc.total_amount, MIN(cc.receive_date) as receive_date FROM civicrm_contribution cc LEFT JOIN civicrm_participant_payment p ON p.contribution_id = cc.id LEFT JOIN civicrm_membership_payment m ON m.contribution_id = cc.id WHERE cc.is_test = 0 AND cc.contribution_status_id = 1 AND p.id IS NULL AND m.id IS NULL AND cc.payment_processor_id IS NOT NULL AND cc.receive_date > '1990-01-01 00:00:00' AND EXTRACT(HOUR_SECOND FROM cc.receive_date) != '000000' AND contribution_recur_id IS NOT NULL GROUP BY cc.contribution_recur_id ) cf GROUP BY WEEKDAY(cf.receive_date)");
          civicrm_statistics_normalize($rows, 7);
          break;
        case 'ccb.r':
          $rows = civicrm_statistics_query("
          select t.idx, count(*) counting, round(sum(t.total_amount)) summ, round(avg(t.total_amount)) avgg
          from (
          select case
          when total_amount between 0 and 100 then 100
          when total_amount between 100 and 500 then 500
          when total_amount between 500 and 1000 then 1000
          when total_amount between 1000 and 2000 then 2000
          when total_amount between 2000 and 5000 then 5000
          when total_amount between 5000 and 10000 then 10000
          when total_amount between 10000 and 20000 then 20000
          else 20001
          end idx,
          total_amount
          from civicrm_contribution
          where contribution_status_id = 1 AND is_test != 1 AND payment_processor_id IS NOT NULL
          ) t
          group by t.idx
          ");
          break;
        case 'cmeo.h':
          $rows = civicrm_statistics_query("SELECT HOUR(time_stamp) idx, COUNT(time_stamp) as counting FROM civicrm_mailing_event_opened group by HOUR(time_stamp)");
          civicrm_statistics_normalize($rows, 24);
          break;
        case 'cmeo.w':
          $rows = civicrm_statistics_query("SELECT WEEKDAY(time_stamp) idx, COUNT(time_stamp) as counting FROM civicrm_mailing_event_opened group by WEEKDAY(time_stamp)");
          civicrm_statistics_normalize($rows, 7);
          break;
        case 'cmeo.n':
          $rows = civicrm_statistics_query("SELECT cc.counting as idx, count(cc.counting) as counting FROM (SELECT event_queue_id, count(event_queue_id) as counting FROM civicrm_mailing_event_opened group by event_queue_id having count(event_queue_id) > 1) as cc GROUP BY cc.counting");
          break;
        case 'cmet.h':
          $rows = civicrm_statistics_query("SELECT HOUR(time_stamp) idx, COUNT(time_stamp) as counting FROM civicrm_mailing_event_trackable_url_open group by HOUR(time_stamp)");
          civicrm_statistics_normalize($rows, 24);
          break;
        case 'cmet.w':
          $rows = civicrm_statistics_query("SELECT WEEKDAY(time_stamp) idx, COUNT(time_stamp) as counting FROM civicrm_mailing_event_trackable_url_open group by WEEKDAY(time_stamp)");
          civicrm_statistics_normalize($rows, 7);
          break;
        case 'cmet.n':
          $rows = civicrm_statistics_query("SELECT cc.counting as idx, count(cc.counting) as counting FROM (SELECT event_queue_id, count(event_queue_id) as counting FROM civicrm_mailing_event_trackable_url_open group by event_queue_id having count(event_queue_id) > 1) as cc GROUP BY cc.counting");
          break;
      }
    }
    else{
      switch($t){
        case 'cc':
          $rows = CRM_Core_DAO::singleValueQuery("SELECT COUNT(*) FROM $table WHERE is_deleted != 1");
          break;
        default:
          $has_test_field = CRM_Core_DAO::singleValueQuery("SHOW COLUMNS FROM $table LIKE 'is_test'");
          if($has_test_field){
            $rows = CRM_Core_DAO::singleValueQuery("SELECT COUNT(*) FROM $table WHERE is_test != 1");
          }
          else{
            $rows = CRM_Core_DAO::singleValueQuery("SELECT COUNT(*) FROM $table WHERE 1");
          }
          break;
      }
    }

    $url = civicrm_statistics_build_fetch_url($sitekey, $t);
    if($debug){
      $stat[$t] = $rows;
    }
    else{
      $client = \Drupal::httpClient();
      $client->post($url, [
        'form_params' => [
          'stat' => json_encode($rows),
        ]
      ]);
      sleep(mt_rand(1, 3));
    }
  }

  if(!empty($stat) && !$debug){
    \Drupal::state()->set('civicrm_statistics.last_cron', $_SERVER['REQUEST_TIME'] + 300*mt_rand(2, 20));
  }
  elseif($debug){
    print_r($stat);
  }
}

function civicrm_statistics_build_fetch_url($sitekey, $type, $year = NULL){
  $base = 'https://neticrm.tw/neticrm/statistics';
  if ($type == 'annual') {
    return $base."/fetch?sitekey={$sitekey}&type={$type}&year={$year}";
  }
  else {
    return $base."/fetch?sitekey={$sitekey}&type={$type}";
  }
}


function civicrm_statistics_table($flip = FALSE){
  $tables = array(
    'civicrm_contact' => 'cc',
    'civicrm_contact.gender' => 'cc.g',
    'civicrm_contact.birth_date' => 'cc.b',
    'civicrm_address.province' => 'ca.p',
    'civicrm_contribution' => 'ccb',
    'civicrm_contribution.amount' => 'ccb.a',
    'civicrm_contribution.hour' => 'ccb.h',
    'civicrm_contribution.week' => 'ccb.w',
    'civicrm_contribution.range' => 'ccb.r',
    'civicrm_mailing_event_delivered' => 'cmed',
    'civicrm_mailing_event_opened' => 'cmeo',
    'civicrm_mailing_event_opened.hour' => 'cmeo.h',
    'civicrm_mailing_event_opened.week' => 'cmeo.w',
    'civicrm_mailing_event_opened.num' => 'cmeo.n',
    'civicrm_mailing_event_trackable_url_open' => 'cmet',
    'civicrm_mailing_event_trackable_url_open.hour' => 'cmet.h',
    'civicrm_mailing_event_trackable_url_open.week' => 'cmet.w',
    'civicrm_mailing_event_trackable_url_open.num' => 'cmet.n',
    'civicrm_event' => 'ce',
    'civicrm_participant' => 'cp',
  );
  if($flip){
    return array_flip($tables);
  }
  return $tables;
}

function civicrm_statistics_query($query){
  $q = CRM_Core_DAO::executeQuery($query);
  $result = array();
  while($q->fetch()){
    if(isset($q->summ)){
      $result[$q->idx] = array(
        'count' => $q->counting,
        'sum' => $q->summ,
        'avg' => $q->avgg,
      );
    }
    else{
      $result[$q->idx] = $q->counting;
    }
  }
  return $result;
}
function civicrm_statistics_normalize(&$rows, $num){
  $null = reset($rows);
  if(is_array($null)){
    foreach($null as $k => $v){
      $null[$k] = 0;
    }
  }
  else{
    $null = 0;
  }
  $clone = $rows;
  $rows = array();
  for($i = 0; $i < $num; $i++){
    if(!isset($clone[$i])){
      $rows[$i] = $null;
    }
    else{
      $rows[$i] = $clone[$i];
    }
  }
}

function civicrm_statistics_annual_fetch($base_year, $force = FALSE) {
  civicrm_initialize();
  $result = CRM_Core_BAO_Cache::getItem('civicrm_statistics_annual', $base_year, NULL, REQUEST_TIME - 86400);
  if (empty($result)) {
    $result = civicrm_statistics_annual_data($base_year);
    CRM_Core_BAO_Cache::setItem($result, 'civicrm_statistics_annual', $base_year);
  }

  if (!empty($result)) {
    $sitekey = $_SERVER['HTTP_HOST'];
    $url = civicrm_statistics_build_fetch_url($sitekey, 'annual', $base_year);
    $client = \Drupal::httpClient();
    $client->post($url, [
      'form_params' => [
        'stat' => json_encode($result),
      ]
    ]);
  }

  return $result;
}

function civicrm_statistics_annual_data($base_year, $run_only = array()) {
  $tables = civicrm_statistics_annual_table();
  $last_year = (int) $base_year - 1;
  $before_last_year = $last_year - 1;
  $last_year = (string) $last_year;

  $result = array();
  foreach($tables as $table => $t){
    if (!empty($run_only)) {
      if (!in_array($table, $run_only)) {
        continue;
      }
    }

    switch($table) {
      case 'contact':
        $params = array(
          'name' => $table,
          'sql' => "SELECT count(*) as `count` FROM (SELECT entity_id, MIN(modified_date) as join_date FROM civicrm_log WHERE entity_table = 'civicrm_contact' GROUP BY entity_id) as {$t['table_alias']} WHERE %start AND %end",
          'years' => array($base_year, $last_year, $before_last_year),
          'table' => $t,
          'fields' => array('count'),
        );
        civicrm_statistics_annual_query_result($result, $params);
        break;

      case 'mailing':
        foreach(array($base_year, $last_year) as $y) {
          $sql = civicrm_statistics_annual_query_modify($t, $y, "SELECT mailing_id FROM civicrm_mailing_job WHERE status = 'Complete' AND is_test = 0 AND parent_id IS NULL AND %start AND %end");
          $query = CRM_Core_DAO::executeQuery($sql);
          $leveling = array(
            '100-500' => 0,
            '500-1000' => 0,
            '1000-5000' => 0,
            '5000-10000' => 0,
            '10000' => 0,
          );
          $num = array(
            'delivered' => $leveling,
            'opened' => $leveling,
            'url_opened' => $leveling,
            'opened_rate' => $leveling,
            'clicked_rate' => $leveling,
          );
          while($query->fetch()) {
            if ($query->mailing_id) {
              $delivered = CRM_Mailing_Event_BAO_Delivered::getTotalCount($query->mailing_id, NULL);
              $opened = CRM_Mailing_Event_BAO_Opened::getTotalCount($query->mailing_id, NULL, TRUE);
              $url_opened = CRM_Mailing_Event_BAO_TrackableURLOpen::getTotalCount($query->mailing_id, NULL, TRUE);
              if ($delivered < 100) continue;
              elseif ($delivered <= 500) $level = '100-500';
              elseif ($delivered <= 1000) $level = '500-1000';
              elseif ($delivered <= 5000) $level = '1000-5000';
              elseif ($delivered <= 10000) $level = '5000-10000';
              elseif ($delivered > 10000) $level = '10000';

              $num['delivered'][$level] += $delivered;
              $num['opened'][$level] += $opened;
              $num['url_opened'][$level] += $url_opened;
            }
          }
          if (!empty($num)) {
            foreach($num['delivered'] as $level => $val) {
              $val = !empty($val) ? $val : 0;
              $num['opened_rate'][$level] = $num['opened'][$level] / $val;
              $num['clicked_rate'][$level] = $num['url_opened'][$level] / $val;
            }
            foreach($num as $type => $level) {
              $result['mailing_'.$type.'_'.$y] = $level;
            }
          }
        }
        break;

      case 'contribution':
        $params = array(
          'name' => $table,
          'sql' => "SELECT count(*) as `count`, sum(total_amount) as `sum` FROM civicrm_contribution cc LEFT JOIN civicrm_participant_payment p ON p.contribution_id = cc.id LEFT JOIN civicrm_membership_payment m ON m.contribution_id = cc.id WHERE cc.is_test = 0 AND cc.contribution_status_id = 1 AND p.id IS NULL AND m.id IS NULL AND %start AND %end",
          'years' => array($base_year, $last_year),
          'table' => $t,
          'fields' => array('count', 'sum'),
        );
        civicrm_statistics_annual_query_result($result, $params);

        $params = array(
          'name' => $table,
          'sql' => "SELECT AVG(c.annual_amount) as avg_contact FROM (SELECT SUM(total_amount) as annual_amount FROM civicrm_contribution cc LEFT JOIN civicrm_participant_payment p ON p.contribution_id = cc.id LEFT JOIN civicrm_membership_payment m ON m.contribution_id = cc.id WHERE cc.is_test = 0 AND cc.contribution_status_id = 1 AND p.id IS NULL AND m.id IS NULL AND %start AND %end GROUP BY cc.contact_id) c",
          'years' => array($base_year, $last_year),
          'table' => $t,
          'fields' => array('avg_contact'),
        );
        civicrm_statistics_annual_query_result($result, $params);
        break;

      case 'contribution_retention':
        $sql = "
SELECT
  last_year.contact_id as contact_id_last,
  base_year.contact_id as contact_id_base,
  last_year.recur as recur_last,
  base_year.recur as recur_base,
  base_year.contribution_count as contribution_count_base,
  base_year.contribution_sum as contribution_sum_base,
  last_year.contribution_count as contribution_count_last,
  last_year.contribution_sum as contribution_sum_last
FROM
  (SELECT cc.contact_id as contact_id, GROUP_CONCAT(cc.contribution_recur_id) as recur, COUNT(cc.id) as contribution_count, SUM(cc.total_amount) as contribution_sum  FROM civicrm_contribution cc LEFT JOIN civicrm_participant_payment p ON p.contribution_id = cc.id LEFT JOIN civicrm_membership_payment m ON m.contribution_id = cc.id WHERE cc.is_test = 0 AND cc.contribution_status_id = 1 AND p.id IS NULL AND m.id IS NULL AND %last_start AND %last_end GROUP BY cc.contact_id) as last_year
LEFT JOIN
  (SELECT cc.contact_id as contact_id, GROUP_CONCAT(cc.contribution_recur_id) as recur, COUNT(cc.id) as contribution_count, SUM(cc.total_amount) as contribution_sum FROM civicrm_contribution cc LEFT JOIN civicrm_participant_payment p ON p.contribution_id = cc.id LEFT JOIN civicrm_membership_payment m ON m.contribution_id = cc.id WHERE cc.is_test = 0 AND cc.contribution_status_id = 1 AND p.id IS NULL AND m.id IS NULL AND %start AND %end GROUP BY cc.contact_id) as base_year
ON base_year.contact_id = last_year.contact_id
";
        $sql = "CREATE TEMPORARY TABLE IF NOT EXISTS {$table} (INDEX contact_id_last (contact_id_last, contact_id_base)) $sql";
        $sql = civicrm_statistics_annual_query_modify($t, $base_year, $sql);
        $sql = civicrm_statistics_annual_query_modify($t, $last_year, $sql, array('%last_start', '%last_end'));
        CRM_Core_DAO::executeQuery($sql);

        $sql = "SELECT COUNT(*) FROM $table WHERE contact_id_last IS NOT NULL"; // last year
        $first = CRM_Core_DAO::singleValueQuery($sql);
        $sql = "SELECT COUNT(*) FROM $table WHERE contact_id_last IS NOT NULL AND contact_id_base IS NOT NULL"; // last year and current (base) year
        $second = CRM_Core_DAO::singleValueQuery($sql);
        $sql = "SELECT COUNT(*) FROM $table WHERE contact_id_last IS NOT NULL AND contact_id_base IS NOT NULL AND recur_base IS NOT NULL AND recur_last IS NOT NULL AND recur_base = recur_last AND contribution_count_base <= 12"; // same recur last and base year, and not 12 month
        $recurring = CRM_Core_DAO::singleValueQuery($sql);
        $result[$table.'_'.$base_year] = ($second - $recurring);
        $result[$table.'_'.$last_year] = $first;
        $result[$table.'_overlap_'.$base_year] = $recurring;

        // amount
        $first = $second = $recurring = NULL;
        $sql = "SELECT SUM(contribution_sum_last) FROM $table WHERE contact_id_last IS NOT NULL"; // last year
        $first = CRM_Core_DAO::singleValueQuery($sql);
        $sql = "SELECT SUM(contribution_sum_base) FROM $table WHERE contact_id_last IS NOT NULL AND contact_id_base IS NOT NULL"; // base year
        $second = CRM_Core_DAO::singleValueQuery($sql);
        $sql = "SELECT SUM(contribution_sum_base) FROM $table WHERE contact_id_last IS NOT NULL AND contact_id_base IS NOT NULL AND recur_base IS NOT NULL AND recur_last IS NOT NULL AND recur_base = recur_last AND contribution_count_base <= 12"; // same recur last and base year, and not 12 month
        $recurring = CRM_Core_DAO::singleValueQuery($sql);
        $result[$table.'_amount_'.$base_year] = ($second - $recurring);
        $result[$table.'_amount_'.$last_year] = $first;
        $result[$table.'_amount_overlap_'.$base_year] = $recurring;


        // lifetime
        $sql = "
SELECT
  c.contact_id, COUNT(c.year) AS num_years,
  SUM(c.count) AS num_contributions,
  SUM(c.summ) as sum_contributions,
  (SELECT total_amount FROM civicrm_contribution WHERE id = MIN(c.first_contribution_id)) as first_contribution_amount,
  1 as grouping
FROM
  ( SELECT
    cc.contact_id,
    YEAR(cc.receive_date) AS year,
    COUNT(cc.id) AS count, SUM(cc.total_amount) AS summ,
    MIN(cc.id) as first_contribution_id
  FROM civicrm_contribution cc
    LEFT JOIN civicrm_participant_payment p ON p.contribution_id = cc.id
    LEFT JOIN civicrm_membership_payment m ON m.contribution_id = cc.id
  WHERE
    cc.is_test = 0 AND cc.contribution_status_id = 1 AND p.id IS NULL AND m.id IS NULL AND cc.receive_date > '2000-01-01' AND %end
  GROUP BY cc.contact_id, YEAR(cc.receive_date)
  ) c
GROUP BY c.contact_id
";
        $sql = civicrm_statistics_annual_query_modify($t, $base_year, $sql);
        $sql = "CREATE TEMPORARY TABLE IF NOT EXISTS {$table}_lifetime (INDEX contact_id (contact_id, num_years, num_contributions)) $sql";
        CRM_Core_DAO::executeQuery($sql);

        $sql = "SELECT COUNT(*) FROM {$table}_lifetime WHERE num_years > 1 OR num_contributions > 12";
        $lifetime_donors_return = CRM_Core_DAO::singleValueQuery($sql);
        $sql = "SELECT COUNT(*) FROM {$table}_lifetime WHERE 1"; // all contacts
        $lifetime_donors = CRM_Core_DAO::singleValueQuery($sql);
        $result[$table.'_lifetime_'.$base_year] = $lifetime_donors_return;
        $result[$table.'_lifetime_'.$last_year] = $lifetime_donors;

        // lifetime amount
        $sql = "SELECT SUM(sum_contributions) as return_amount, SUM(first_contribution_amount) as first_amount FROM {$table}_lifetime WHERE num_years > 1 OR num_contributions > 12 GROUP BY grouping";
        $query = CRM_Core_DAO::executeQuery($sql);
        $query->fetch();
        $sql = "SELECT SUM(sum_contributions) FROM {$table}_lifetime WHERE 1"; // all contacts
        $lifetime_amount = CRM_Core_DAO::singleValueQuery($sql);
        $result[$table.'_amount_lifetime_'.$base_year] = $query->return_amount - $query->first_amount;
        $result[$table.'_amount_lifetime_'.$last_year] = $lifetime_amount;
        $result[$table.'_amount_lifetime_first_'.$base_year] = $query->first_amount;

        // first time donor percentage
        $sql = "
SELECT COUNT(donors.new) as new, COUNT(donors.returned) as returned, (COUNT(donors.new)/COUNT(donors.returned)) as rate FROM (
  SELECT cc.contact_id AS returned, c.contact_id AS new, cc.contribution_status_id FROM civicrm_contribution cc
    LEFT JOIN civicrm_participant_payment p ON p.contribution_id = cc.id
    LEFT JOIN civicrm_membership_payment m ON m.contribution_id = cc.id
    LEFT JOIN ( SELECT id, contact_id FROM civicrm_contribution GROUP BY contact_id HAVING MIN(receive_date) >= '%min' ) c ON cc.id = c.id
  WHERE %start AND %end AND cc.is_test = 0 AND cc.contribution_status_id = 1 AND p.id IS NULL AND m.id IS NULL
  GROUP BY cc.contact_id
) AS donors
GROUP BY donors.contribution_status_id        
";
        $basesql = civicrm_statistics_annual_query_modify($t, $base_year, $sql);
        $basesql = str_replace('%min', $base_year.'-01-01 00:00:00', $basesql);
        $query = CRM_Core_DAO::executeQuery($basesql);
        $query->fetch();
        $result[$table.'_donor_new_'.$base_year] = $query->new;
        $result[$table.'_donor_returned_'.$base_year] = $query->returned;
        $result[$table.'_donor_new_rate_'.$base_year] = $query->rate;

        $lastsql = civicrm_statistics_annual_query_modify($t, $last_year, $sql);
        $lastsql = str_replace('%min', $last_year.'-01-01 00:00:00', $lastsql);
        $query = CRM_Core_DAO::executeQuery($lastsql);
        $query->fetch();
        $result[$table.'_donor_new_'.$last_year] = $query->new;
        $result[$table.'_donor_returned_'.$last_year] = $query->returned;
        $result[$table.'_donor_new_rate_'.$last_year] = $query->rate;

        // drop temp table at the end
        CRM_Core_DAO::executeQuery("DROP TEMPORARY TABLE {$table}");
        CRM_Core_DAO::executeQuery("DROP TEMPORARY TABLE {$table}_lifetime");
        break;

      case 'contribution_leveling':
        // amount leveling
        foreach(array($base_year, $last_year) as $y) {
          $sql = "SELECT cc.contact_id, SUM(cc.total_amount) as annual_amount FROM civicrm_contribution cc LEFT JOIN civicrm_participant_payment p ON p.contribution_id = cc.id LEFT JOIN civicrm_membership_payment m ON m.contribution_id = cc.id WHERE cc.is_test = 0 AND cc.contribution_status_id = 1 AND p.id IS NULL AND m.id IS NULL AND %start AND %end GROUP BY cc.contact_id";
          $sql = civicrm_statistics_annual_query_modify($t, $y, $sql);
          $sql = "CREATE TEMPORARY TABLE IF NOT EXISTS {$table}_{$y} (INDEX annual_amount (annual_amount)) $sql";
          CRM_Core_DAO::executeQuery($sql);
          $sql = "
            SELECT c.level, count(*) as `count` FROM  (
              SELECT CASE
              when annual_amount between 0 and 1000 then 1000
              when annual_amount between 1000 and 5000 then 5000
              when annual_amount between 5000 and 10000 then 10000
              when annual_amount between 10000 and 20000 then 20000
              ELSE 20001
              END level FROM {$table}_{$y}
            ) c
            GROUP BY c.level ORDER BY c.level";
          $query = CRM_Core_DAO::executeQuery($sql);
          while($query->fetch()) {
            $result[$table.'_'.$y][$query->level] = $query->count;
          }
          CRM_Core_DAO::executeQuery("DROP TEMPORARY TABLE {$table}_{$y}");
        }

        // non recur amount leveling
        foreach(array($base_year, $last_year) as $y) {
          $sql = "SELECT cc.contact_id, SUM(cc.total_amount) as annual_amount FROM civicrm_contribution cc LEFT JOIN civicrm_participant_payment p ON p.contribution_id = cc.id LEFT JOIN civicrm_membership_payment m ON m.contribution_id = cc.id WHERE cc.is_test = 0 AND cc.contribution_status_id = 1 AND p.id IS NULL AND m.id IS NULL AND cc.contribution_recur_id IS NULL AND %start AND %end GROUP BY cc.contact_id";
          $sql = civicrm_statistics_annual_query_modify($t, $y, $sql);
          $sql = "CREATE TEMPORARY TABLE IF NOT EXISTS {$table}_nonrecur_{$y} (INDEX annual_amount (annual_amount)) $sql";
          CRM_Core_DAO::executeQuery($sql);
          $sql = "
            SELECT c.level, count(*) as `count` FROM  (
              SELECT CASE
              when annual_amount between 0 and 1000 then 1000
              when annual_amount between 1000 and 5000 then 5000
              when annual_amount between 5000 and 10000 then 10000
              when annual_amount between 10000 and 20000 then 20000
              ELSE 20001
              END level FROM {$table}_nonrecur_{$y}
            ) c
            GROUP BY c.level ORDER BY c.level";
          $query = CRM_Core_DAO::executeQuery($sql);
          while($query->fetch()) {
            $result[$table.'_nonrecur_'.$y][$query->level] = $query->count;
          }
          CRM_Core_DAO::executeQuery("DROP TEMPORARY TABLE {$table}_nonrecur_{$y}");
        }

        // leveling by age
        foreach(array($base_year, $last_year) as $y) {
          $sql = "SELECT cc.contact_id, YEAR(CURRENT_TIMESTAMP) - YEAR(c.birth_date) - (RIGHT(CURRENT_TIMESTAMP, 5) < RIGHT(c.birth_date, 5)) as age FROM civicrm_contribution cc INNER JOIN civicrm_contact c ON c.id = cc.contact_id LEFT JOIN civicrm_participant_payment p ON p.contribution_id = cc.id LEFT JOIN civicrm_membership_payment m ON m.contribution_id = cc.id WHERE cc.is_test = 0 AND cc.contribution_status_id = 1 AND p.id IS NULL AND m.id IS NULL AND c.birth_date IS NOT NULL AND %start AND %end GROUP BY cc.contact_id";
          $sql = civicrm_statistics_annual_query_modify($t, $y, $sql);
          $sql = "CREATE TEMPORARY TABLE IF NOT EXISTS {$table}_age_{$y} (INDEX age (age)) $sql";
          CRM_Core_DAO::executeQuery($sql);
          $sql = "
            SELECT c.age, count(*) as `count` FROM  (
              SELECT CASE
              when age between 0 and 20 then 20
              when age between 20 and 30 then 30
              when age between 30 and 40 then 40
              when age between 40 and 50 then 50
              when age between 50 and 60 then 60
              when age between 60 and 200 then 61
              ELSE 61
              END age FROM {$table}_age_{$y}
            ) c
            GROUP BY c.age ORDER BY c.age";
          $query = CRM_Core_DAO::executeQuery($sql);
          while($query->fetch()) {
            $result[$table.'_age_'.$y][$query->age] = $query->count;
          }
          CRM_Core_DAO::executeQuery("DROP TEMPORARY TABLE {$table}_age_{$y}");
        }
        foreach(array($base_year, $last_year) as $y) {
          $sql = "SELECT cc.contact_id, SUM(cc.total_amount) as total_amount, YEAR(CURRENT_TIMESTAMP) - YEAR(c.birth_date) - (RIGHT(CURRENT_TIMESTAMP, 5) < RIGHT(c.birth_date, 5)) as age FROM civicrm_contribution cc INNER JOIN civicrm_contact c ON c.id = cc.contact_id LEFT JOIN civicrm_participant_payment p ON p.contribution_id = cc.id LEFT JOIN civicrm_membership_payment m ON m.contribution_id = cc.id WHERE cc.is_test = 0 AND cc.contribution_status_id = 1 AND p.id IS NULL AND m.id IS NULL AND c.birth_date IS NOT NULL AND %start AND %end GROUP BY cc.contact_id";
          $sql = civicrm_statistics_annual_query_modify($t, $y, $sql);
          $sql = "CREATE TEMPORARY TABLE IF NOT EXISTS {$table}_age_sum_{$y} (INDEX age (age)) $sql";
          CRM_Core_DAO::executeQuery($sql);
          $sql = "
            SELECT c.age, SUM(c.total_amount) as `sum` FROM  (
              SELECT CASE
              when age between 0 and 20 then 20
              when age between 20 and 30 then 30
              when age between 30 and 40 then 40
              when age between 40 and 50 then 50
              when age between 50 and 60 then 60
              when age between 60 and 200 then 61
              ELSE 61
              END age, total_amount FROM {$table}_age_sum_{$y}
            ) c
            GROUP BY c.age ORDER BY c.age";
          $query = CRM_Core_DAO::executeQuery($sql);
          while($query->fetch()) {
            $result[$table.'_age_amount_'.$y][$query->age] = $query->sum;
          }
          CRM_Core_DAO::executeQuery("DROP TEMPORARY TABLE {$table}_age_sum_{$y}");
        }

        // state and province leveling
        $states = CRM_Core_PseudoConstant::stateProvince();
        foreach(array($base_year, $last_year) as $y) {
          $sql = "SELECT f.state_province_id, COUNT(f.id) as count FROM (SELECT c.id, a.state_province_id FROM civicrm_contribution cc INNER JOIN civicrm_contact c ON c.id = cc.contact_id INNER JOIN civicrm_address a ON c.id = a.contact_id AND a.is_primary = 1 LEFT JOIN civicrm_participant_payment p ON p.contribution_id = cc.id LEFT JOIN civicrm_membership_payment m ON m.contribution_id = cc.id WHERE cc.is_test = 0 AND cc.contribution_status_id = 1 AND p.id IS NULL AND m.id IS NULL AND a.state_province_id IS NOT NULL AND %start AND %end GROUP BY cc.contact_id) f GROUP BY f.state_province_id ORDER BY state_province_id";
          $sql = civicrm_statistics_annual_query_modify($t, $y, $sql);
          $query = CRM_Core_DAO::executeQuery($sql);

          while($query->fetch()) {
            $state = $states[$query->state_province_id];
            if (empty($state)) {
              continue;
            }
            $result[$table.'_city_'.$y][$state] = $query->count;
          }
        }

        break;

      case 'contribution_recur':
        $params = array(
          'name' => $table,
          'sql' => "SELECT SUM(total_amount) as `sum`, count(*) as `count` FROM civicrm_contribution cc LEFT JOIN civicrm_participant_payment p ON p.contribution_id = cc.id LEFT JOIN civicrm_membership_payment m ON m.contribution_id = cc.id WHERE cc.is_test = 0 AND cc.contribution_status_id = 1 AND p.id IS NULL AND m.id IS NULL AND cc.contribution_recur_id IS NOT NULL AND %start AND %end",
          'years' => array($base_year, $last_year),
          'table' => $t,
          'fields' => array('sum', 'count'),
        );
        civicrm_statistics_annual_query_result($result, $params);

        $params = array(
          'name' => $table,
          'sql' => "SELECT COUNT(*) as contact_count FROM (SELECT COUNT(*) as `count` FROM civicrm_contribution cc LEFT JOIN civicrm_participant_payment p ON p.contribution_id = cc.id LEFT JOIN civicrm_membership_payment m ON m.contribution_id = cc.id WHERE cc.is_test = 0 AND cc.contribution_status_id = 1 AND p.id IS NULL AND m.id IS NULL AND cc.contribution_recur_id IS NOT NULL AND %start AND %end GROUP BY cc.contact_id) ccc",
          'years' => array($base_year, $last_year),
          'table' => $t,
          'fields' => array('contact_count'),
        );
        civicrm_statistics_annual_query_result($result, $params);
        break;
      case 'contribution_online':
        $params = array(
          'name' => $table,
          'sql' => "SELECT SUM(total_amount) as nonrecur_sum, COUNT(*) as nonrecur_count FROM civicrm_contribution cc LEFT JOIN civicrm_participant_payment p ON p.contribution_id = cc.id LEFT JOIN civicrm_membership_payment m ON m.contribution_id = cc.id WHERE cc.is_test = 0 AND cc.contribution_status_id = 1 AND p.id IS NULL AND m.id IS NULL AND cc.contribution_recur_id IS NULL AND cc.payment_processor_id IS NOT NULL AND %start AND %end GROUP BY cc.contribution_status_id",
          'years' => array($base_year, $last_year),
          'table' => $t,
          'fields' => array('nonrecur_sum', 'nonrecur_count'),
        );
        civicrm_statistics_annual_query_result($result, $params);

        $params = array(
          'name' => $table,
          'sql' => "SELECT SUM(cc.total_amount) as recur_sum, COUNT(*) as recur_count FROM civicrm_contribution cc LEFT JOIN civicrm_participant_payment p ON p.contribution_id = cc.id LEFT JOIN civicrm_membership_payment m ON m.contribution_id = cc.id WHERE cc.is_test = 0 AND cc.contribution_status_id = 1 AND p.id IS NULL AND m.id IS NULL AND cc.contribution_recur_id IS NOT NULL AND cc.payment_processor_id IS NOT NULL AND %start AND %end GROUP BY cc.contribution_status_id",
          'years' => array($base_year, $last_year),
          'table' => $t,
          'fields' => array('recur_sum', 'recur_count'),
        );
        civicrm_statistics_annual_query_result($result, $params);

        $instruments = CRM_Contribute_PseudoConstant::paymentInstrument();
        foreach(array($base_year, $last_year) as $y) {
          $sql = "SELECT MONTH(cf.receive_date) as `month`, SUM(cf.total_amount) as `sum` FROM (SELECT cc.id, cc.total_amount, cc.receive_date as receive_date FROM civicrm_contribution cc LEFT JOIN civicrm_participant_payment p ON p.contribution_id = cc.id LEFT JOIN civicrm_membership_payment m ON m.contribution_id = cc.id WHERE cc.is_test = 0 AND cc.contribution_status_id = 1 AND p.id IS NULL AND m.id IS NULL AND cc.payment_processor_id IS NOT NULL AND cc.contribution_recur_id IS NULL AND %start AND %end ) cf GROUP BY MONTH(cf.receive_date) ORDER BY MONTH(cf.receive_date)";
          $sql = civicrm_statistics_annual_query_modify($t, $y, $sql);
          $query = CRM_Core_DAO::executeQuery($sql);
          while($query->fetch()) {
            $result[$table."_monthly_".$y][$query->month] = $query->sum;
          }

          $sql = "SELECT MONTH(ccc.receive_date) as `month`, SUM(total_amount) as `sum` FROM (SELECT cc.total_amount, cc.receive_date FROM civicrm_contribution cc LEFT JOIN civicrm_participant_payment p ON p.contribution_id = cc.id LEFT JOIN civicrm_membership_payment m ON m.contribution_id = cc.id WHERE cc.is_test = 0 AND cc.contribution_status_id = 1 AND p.id IS NULL AND m.id IS NULL AND cc.contribution_recur_id IS NOT NULL AND cc.payment_processor_id IS NOT NULL AND %start AND %end) ccc GROUP BY MONTH(ccc.receive_date)";
          $sql = civicrm_statistics_annual_query_modify($t, $y, $sql);
          $query = CRM_Core_DAO::executeQuery($sql);
          while($query->fetch()) {
            $result[$table."_monthly_recur_".$y][$query->month] = $query->sum;
          }

          $sql = "SELECT cc.payment_instrument_id, SUM(total_amount) as `sum` FROM civicrm_contribution cc LEFT JOIN civicrm_participant_payment p ON p.contribution_id = cc.id LEFT JOIN civicrm_membership_payment m ON m.contribution_id = cc.id WHERE cc.is_test = 0 AND cc.contribution_status_id = 1 AND p.id IS NULL AND m.id IS NULL AND cc.payment_processor_id IS NOT NULL AND cc.payment_instrument_id IS NOT NULL AND %start AND %end GROUP BY cc.payment_instrument_id";
          $sql = civicrm_statistics_annual_query_modify($t, $y, $sql);
          $query = CRM_Core_DAO::executeQuery($sql);
          while($query->fetch()) {
            $label = $query->payment_instrument_id."-".$instruments[$query->payment_instrument_id];
            $result[$table."_instrument_".$y][$label] = $query->sum;
          }
        }
        break;
      case 'event':
        $et = $t;
        $et['date_field'] = 'start_date';
        $et['table_alias'] = 'e';
        $params = array(
          'name' => 'event',
          'sql' => "SELECT COUNT(*) as `count` FROM civicrm_event e WHERE %start AND %end",
          'years' => array($base_year, $last_year),
          'table' => $et,
          'fields' => array('count'),
        );
        civicrm_statistics_annual_query_result($result, $params);

        $params = array(
          'name' => 'event_participant',
          'sql' => "SELECT COUNT(*) as `count` FROM civicrm_participant p WHERE p.status_id = 1 AND p.is_test = 0 AND %start AND %end",
          'years' => array($base_year, $last_year),
          'table' => $t,
          'fields' => array('count'),
        );
        civicrm_statistics_annual_query_result($result, $params);


        $params = array(
          'name' => 'event_participant_free',
          'sql' => "SELECT COUNT(*) as `count` FROM civicrm_participant p INNER JOIN civicrm_event e ON e.id = p.event_id WHERE p.status_id = 1 AND p.is_test = 0 AND e.is_monetary = 0 AND %start AND %end",
          'years' => array($base_year, $last_year),
          'table' => $t,
          'fields' => array('count'),
        );
        civicrm_statistics_annual_query_result($result, $params);

        $params = array(
          'name' => 'event_participant_monetary',
          'sql' => "SELECT COUNT(*) as `count` FROM civicrm_participant p INNER JOIN civicrm_event e ON e.id = p.event_id WHERE p.status_id = 1 AND p.is_test = 0 AND e.is_monetary = 1 AND %start AND %end",
          'years' => array($base_year, $last_year),
          'table' => $t,
          'fields' => array('count'),
        );
        civicrm_statistics_annual_query_result($result, $params);
        break;
      case 'trend_contribution':
        $sql = "SELECT HOUR(cf.receive_date) as `hour`, SUM(cf.total_amount) as `sum` FROM (SELECT cc.id, cc.total_amount, cc.receive_date as receive_date FROM civicrm_contribution cc LEFT JOIN civicrm_participant_payment p ON p.contribution_id = cc.id LEFT JOIN civicrm_membership_payment m ON m.contribution_id = cc.id WHERE cc.is_test = 0 AND cc.contribution_status_id = 1 AND p.id IS NULL AND m.id IS NULL AND cc.payment_processor_id IS NOT NULL AND cc.receive_date > '1990-01-01 00:00:00' AND EXTRACT(HOUR_SECOND FROM cc.receive_date) != '000000' AND cc.contribution_recur_id IS NULL AND %start AND %end UNION SELECT cc.id, cc.total_amount, MIN(cc.receive_date) as receive_date FROM civicrm_contribution cc LEFT JOIN civicrm_participant_payment p ON p.contribution_id = cc.id LEFT JOIN civicrm_membership_payment m ON m.contribution_id = cc.id WHERE cc.is_test = 0 AND cc.contribution_status_id = 1 AND p.id IS NULL AND m.id IS NULL AND cc.payment_processor_id IS NOT NULL AND cc.receive_date > '1990-01-01 00:00:00' AND EXTRACT(HOUR_SECOND FROM cc.receive_date) != '000000' AND contribution_recur_id IS NOT NULL AND %start AND %end GROUP BY cc.contribution_recur_id ) cf GROUP BY HOUR(cf.receive_date) ORDER BY HOUR(cf.receive_date)";
        $params = array(
          'name' => $table,
          'sql' => $sql,
          'years' => array($base_year),
          'table' => $t,
          'fields' => array('hour' => 'sum'),
          'allrows' => 24,
        );
        civicrm_statistics_annual_query_result($result, $params);

        $sql = "SELECT WEEKDAY(cf.receive_date) as `week`, SUM(cf.total_amount) as `sum` FROM (SELECT cc.id, cc.total_amount, cc.receive_date as receive_date FROM civicrm_contribution cc LEFT JOIN civicrm_participant_payment p ON p.contribution_id = cc.id LEFT JOIN civicrm_membership_payment m ON m.contribution_id = cc.id WHERE cc.is_test = 0 AND cc.contribution_status_id = 1 AND p.id IS NULL AND m.id IS NULL AND cc.payment_processor_id IS NOT NULL AND cc.receive_date > '1990-01-01 00:00:00' AND EXTRACT(HOUR_SECOND FROM cc.receive_date) != '000000' AND cc.contribution_recur_id IS NULL AND %start AND %end UNION SELECT cc.id, cc.total_amount, MIN(cc.receive_date) as receive_date FROM civicrm_contribution cc LEFT JOIN civicrm_participant_payment p ON p.contribution_id = cc.id LEFT JOIN civicrm_membership_payment m ON m.contribution_id = cc.id WHERE cc.is_test = 0 AND cc.contribution_status_id = 1 AND p.id IS NULL AND m.id IS NULL AND cc.payment_processor_id IS NOT NULL AND cc.receive_date > '1990-01-01 00:00:00' AND EXTRACT(HOUR_SECOND FROM cc.receive_date) != '000000' AND contribution_recur_id IS NOT NULL AND %start AND %end GROUP BY cc.contribution_recur_id ) cf GROUP BY WEEKDAY(cf.receive_date) ORDER BY WEEKDAY(cf.receive_date)";
        $params = array(
          'name' => $table,
          'sql' => $sql,
          'years' => array($base_year),
          'table' => $t,
          'fields' => array('week' => 'sum'),
          'allrows' => 7,
        );
        civicrm_statistics_annual_query_result($result, $params);
        break;
      case 'trend_mailing':
        $sql = "SELECT HOUR(time_stamp) as hour, COUNT(time_stamp) as opened FROM civicrm_mailing_event_opened m WHERE %start AND %end GROUP BY HOUR(time_stamp)";
        $params = array(
          'name' => $table,
          'sql' => $sql,
          'years' => array($base_year),
          'table' => $t,
          'fields' => array('hour' => 'opened'),
          'allrows' => 24,
        );
        civicrm_statistics_annual_query_result($result, $params);

        $sql = "SELECT WEEKDAY(time_stamp) as week, COUNT(time_stamp) as opened FROM civicrm_mailing_event_opened m WHERE %start AND %end GROUP BY WEEKDAY(time_stamp)";
        $params = array(
          'name' => $table,
          'sql' => $sql,
          'years' => array($base_year),
          'table' => $t,
          'fields' => array('week' => 'opened'),
          'allrows' => 7,
        );
        civicrm_statistics_annual_query_result($result, $params);
        break;
      case 'trend_event':
        $sql = "SELECT HOUR(register_date) as hour, COUNT(*) as registered FROM civicrm_participant p WHERE %start AND %end GROUP BY HOUR(register_date)";
        $params = array(
          'name' => $table,
          'sql' => $sql,
          'years' => array($base_year),
          'table' => $t,
          'fields' => array('hour' => 'registered'),
          'allrows' => 24,
        );
        civicrm_statistics_annual_query_result($result, $params);

        $sql = "SELECT WEEKDAY(register_date) as week, COUNT(*) as registered FROM civicrm_participant p WHERE %start AND %end GROUP BY WEEKDAY(register_date)";
        $params = array(
          'name' => $table,
          'sql' => $sql,
          'years' => array($base_year),
          'table' => $t,
          'fields' => array('week' => 'registered'),
          'allrows' => 7,
        );
        civicrm_statistics_annual_query_result($result, $params);
        break;

      case 'track':
        $sql = "SELECT t.referrer_type, COUNT(t.id) as `count` FROM civicrm_track t WHERE %start AND %end AND t.page_type = 'civicrm_contribution_page' AND t.referrer_type IS NOT NULL GROUP BY t.referrer_type";
        $params = array(
          'name' => $table.'_referrer_type',
          'sql' => $sql,
          'years' => array($base_year),
          'table' => $t,
          'fields' => array('referrer_type' => 'count'),
          'allrows' => 'all',
        );
        civicrm_statistics_annual_query_result($result, $params);

        $sql = "SELECT t.referrer_type, COUNT(t.id) as `count` FROM civicrm_track t WHERE %start AND %end AND t.page_type = 'civicrm_contribution_page' AND t.referrer_type IS NOT NULL AND t.entity_id IS NOT NULL AND t.state >= 4 GROUP BY t.referrer_type";
        $params = array(
          'name' => $table.'_referrer_type_goal',
          'sql' => $sql,
          'years' => array($base_year),
          'table' => $t,
          'fields' => array('referrer_type' => 'count'),
          'allrows' => 'all',
        );
        civicrm_statistics_annual_query_result($result, $params);

        $sql = "SELECT t.referrer_network, COUNT(t.id) as `count` FROM civicrm_track t WHERE %start AND %end AND t.page_type = 'civicrm_contribution_page' AND t.referrer_type = 'social' AND t.referrer_network IS NOT NULL GROUP BY t.referrer_network";
        $params = array(
          'name' => $table.'_social_network',
          'sql' => $sql,
          'years' => array($base_year),
          'table' => $t,
          'fields' => array('referrer_network' => 'count'),
          'allrows' => 'all',
        );
        civicrm_statistics_annual_query_result($result, $params);

        $sql = "SELECT t.referrer_network, COUNT(t.id) as `count` FROM civicrm_track t WHERE %start AND %end AND t.page_type = 'civicrm_contribution_page' AND t.referrer_type = 'social' AND t.referrer_network IS NOT NULL AND t.entity_id IS NOT NULL AND t.state >= 4 GROUP BY t.referrer_network";
        $params = array(
          'name' => $table.'_social_network_goal',
          'sql' => $sql,
          'years' => array($base_year),
          'table' => $t,
          'fields' => array('referrer_network' => 'count'),
          'allrows' => 'all',
        );
        civicrm_statistics_annual_query_result($result, $params);
    }
  }
  return $result;
}

function civicrm_statistics_annual_table($flip = FALSE){
  $tables = array(
    'contact' => array('table_alias' => 'contact_join', 'date_field' => 'join_date', 'primary_id' => 'entity_id'),
    'mailing' => array('table_alias' => 'civicrm_mailing_job', 'date_field' => 'start_date', 'primary_id' => 'mailing_id'),
    'contribution' => array('table_alias' => 'cc', 'date_field' => 'receive_date', 'primary_id' => 'id'),
    'contribution_retention' => array('table_alias' => 'cc', 'date_field' => 'receive_date', 'primary_id' => 'id'),
    'contribution_leveling' => array('table_alias' => 'cc', 'date_field' => 'receive_date', 'primary_id' => 'id'),
    'contribution_recur' => array('table_alias' => 'cc', 'date_field' => 'receive_date', 'primary_id' => 'id'),
    'contribution_online' => array('table_alias' => 'cc', 'date_field' => 'receive_date', 'primary_id' => 'id'),
    'event' => array('table_alias' => 'p', 'date_field' => 'register_date', 'primary_id' => 'id'),
    'trend_contribution' => array('table_alias' => 'cc', 'date_field' => 'receive_date', 'primary_id' => 'id'),
    'trend_mailing' => array('table_alias' => 'm', 'date_field' => 'time_stamp', 'primary_id' => 'id'),
    'trend_event' => array('table_alias' => 'p', 'date_field' => 'register_date', 'primary_id' => 'id'),
    'track' => array('table_alias' => 't', 'date_field' => 'visit_date', 'primary_id' => 'id'),
  );
  if($flip){
    return array_flip($tables);
  }
  return $tables;
}

function civicrm_statistics_annual_query_result(&$result, $params) {
  foreach($params['years'] as $y) {
    $table = $params['table'];
    $sql = civicrm_statistics_annual_query_modify($table, $y, $params['sql']);
    $query = CRM_Core_DAO::executeQuery($sql);
    if (!empty($params['allrows'])) {
      $group = array();
      while($query->fetch()) {
        foreach($params['fields'] as $k => $f) {
          $idx = $query->$k;
          $result[$params['name'].'_'.$k.'_'.$f.'_'.$y][$idx] = $query->$f;
          $group[] = $params['name'].'_'.$k.'_'.$f.'_'.$y;
        }
      }
      if (!empty($group) && is_numeric($params['allrows'])) {
        foreach($group as $g) {
          civicrm_statistics_normalize($result[$g], $params['allrows']);
        }
      }
    }
    else {
      $query->fetch();
      foreach($params['fields'] as $f) {
        $result[$params['name'].'_'.$f.'_'.$y] = $query->$f;
      }
    }
  }
}

function civicrm_statistics_annual_query_modify($table, $year, $sql, $pattern = array('%start', '%end')) {
  $start = $table['table_alias'].'.'.$table['date_field'].' >= '."'{$year}-01-01 00:00:00'";
  $end = $table['table_alias'].'.'.$table['date_field'].' <= '."'{$year}-12-31 23:59:59'";
  return str_replace($pattern, array($start, $end), $sql);
}
