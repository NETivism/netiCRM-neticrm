<?php

function _neticrm_update_update_x319() {
  civicrm_initialize();
  $updateIndex = [
    'civicrm_mailing' => ['is_hidden'],
    'civicrm_mailing_job' => ['start_date', 'is_test', 'job_type'],
  ];
  foreach($updateIndex as $table => $fields) {
    foreach($fields as $field) {
      $check = "SHOW INDEX FROM {$table} WHERE Key_name = 'index_{$field}'";
      $dao = CRM_Core_DAO::executeQuery($check);
      $dao->fetch();
      // exists
      if ($dao->Key_name == 'index_'.$field) {
        continue;
      }
      // add index
      else {
        $sql = "ALTER TABLE {$table} ADD INDEX index_{$field} ({$field})";
        CRM_Core_DAO::executeQuery($sql);
      }
    }
  }
}