<?php
/**
 * @file
 * Update for 6110
 *
 * This file should be civicrm compliant.
 */
function _neticrm_update_update_6110(){
  // First, clear participant status translations in drupal
  // refs #16766 old problem
  $status = array(
    'Registered',
    'Attended',
    'No-show',
    'Cancelled',
    'Pending from pay later',
    'Pending from incomplete transaction',
    'On waitlist',
    'Awaiting approval',
    'Pending from waitlist',
    'Pending from approval',
    'Rejected',
    'Expired',
  );
  $sql = "SELECT s.lid, CONVERT( s.source
  USING utf8 ) AS source, CONVERT( t.translation
  USING utf8 ) AS translation
  FROM  `locales_source` s
  LEFT JOIN locales_target t ON t.lid = s.lid
  WHERE s.source
  IN ('".implode("','", $status)."')";
  $result = db_query($sql);
  $old = array();
  while($s = db_fetch_object($result)) {
    $old[$s->source] = $s->translation;
    db_query('DELETE FROM {locales_source} WHERE lid = %d', $s->lid);
    db_query('DELETE FROM {locales_target} WHERE lid = %d', $s->lid);
  }
  cache_clear_all('locale:', 'cache', TRUE);

  // Second, update civicrm database. Beware of old translations
  $old = array (
    'Registered' => '巳登記',
    'Attended' => '出席',
    'No-show' => '失約',
    'Cancelled' => '已取消',
    'Pending from pay later' => '待處理-後續付款',
    'Pending from incomplete transaction' => '待處理-未完成交易',
    'On waitlist' => '在候補名單',
    'Awaiting approval' => '等待審核',
    'Pending from waitlist' => '待處理-在候補名單中',
    'Pending from approval' => '待處理-在審核名單中',
    'Rejected' => '已拒絕',
    'Expired' => '已清除',
  );
  foreach($old as $name => $label){
    if(ts($name) != $label){
      $sql = "UPDATE civicrm_participant_status_type SET label = %1 WHERE name = %2 AND label = %3";
      $params = array(
        1 => array(ts($name), 'String'), // new translations
        2 => array($name, 'String'),
        3 => array($label, 'String'),
      );
      CRM_Core_DAO::executeQuery($sql, $params);
    }
  }
  return 'Successful update status';
}
