<?php

function _neticrm_update_update_x320() {

  civicrm_initialize();

  // Add 'is_sync' column to 'civicrm_group' table if it doesn't exist
  $query = "SHOW COLUMNS FROM `civicrm_contact` LIKE 'do_not_notify'";
  $result = CRM_Core_DAO::executeQuery($query);
  if ($result->N == 0) {
    $sql = "ALTER TABLE `civicrm_contact` ADD `do_not_notify` TINYINT NULL DEFAULT '0' COMMENT 'Contact who do not want to receive workflow related notification.' AFTER `do_not_trade`";
    CRM_Core_DAO::executeQuery($sql);
  }
}
