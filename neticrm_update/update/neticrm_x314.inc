<?php

function _neticrm_update_update_x314() {
  $query = "SHOW COLUMNS FROM `civicrm_group` LIKE 'is_sync'";
  $result = CRM_Core_DAO::executeQuery($query);
  if ($result->N == 0) {
    $sql = "ALTER TABLE `civicrm_group` ADD `is_sync` tinyint COMMENT 'Indicates whether the group is synchronized.'";
    CRM_Core_DAO::executeQuery($sql);
  }

  $query = "SHOW COLUMNS FROM `civicrm_group` LIKE 'sync_data'";
  $result = CRM_Core_DAO::executeQuery($query);
  if ($result->N == 0) {
    $sql = "ALTER TABLE `civicrm_group` ADD `sync_data` blob COMMENT 'Stores JSON format data from the remote group, such as remote group ID, name, creation time.'";
    CRM_Core_DAO::executeQuery($sql);
  }

  $query = "SHOW COLUMNS FROM `civicrm_group` LIKE 'last_sync'";
  $result = CRM_Core_DAO::executeQuery($query);
  if ($result->N == 0) {
    $sql = "ALTER TABLE `civicrm_group` ADD `last_sync` datetime COMMENT 'Stores the last sync time with the remote group.'";
    CRM_Core_DAO::executeQuery($sql);
  }
}