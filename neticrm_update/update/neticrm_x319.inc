<?php

function _neticrm_update_update_x319() {
  civicrm_initialize();
  $updateIndex = array(
    'civicrm_mailing' => array('is_hidden'),
    'civicrm_mailing_job' => array('start_date', 'is_test'),
  );
  foreach($updateIndex as $table => $fields) {
    foreach($fields as $field) {
      $check = "SHOW INDEX FROM {$table} WHERE Key_name = 'UI_{$field}'";
      $dao = CRM_Core_DAO::executeQuery($check);
      $dao->fetch();
      // exists
      if ($dao->Key_name == 'UI_'.$field) {
        continue;
      }
      // add index
      else {
        $sql = "ALTER TABLE {$table} ADD INDEX UI_{$field} ({$field})";
        CRM_Core_DAO::executeQuery($sql);
      }
    }
  }
}