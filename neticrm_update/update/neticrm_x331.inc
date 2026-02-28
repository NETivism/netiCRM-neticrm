<?php

function _neticrm_update_update_x331() {

  civicrm_initialize();

  // Add 'source_ip' column to 'civicrm_contribution' table if it doesn't exist
  $query = "SHOW COLUMNS FROM `civicrm_contribution` LIKE 'source_ip'";
  $result = CRM_Core_DAO::executeQuery($query);
  if ($result->N == 0) {
    $sql = "ALTER TABLE `civicrm_contribution` ADD `source_ip` VARCHAR(45) NULL DEFAULT NULL COMMENT 'IP address at the time of contribution creation' AFTER `expire_date`";
    CRM_Core_DAO::executeQuery($sql);
  }
}
