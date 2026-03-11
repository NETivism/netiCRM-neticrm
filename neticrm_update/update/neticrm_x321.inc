<?php

function _neticrm_update_update_x321() {

  civicrm_initialize();

  // Add 'created_id' column to 'civicrm_contribution_tappay' table if it doesn't exist
  $query = "SHOW COLUMNS FROM `civicrm_contribution_tappay` LIKE 'created_id'";
  $result = CRM_Core_DAO::executeQuery($query);
  if ($result->N == 0) {
    $sql = "ALTER TABLE `civicrm_contribution_tappay` ADD `created_id` INT(10) UNSIGNED NULL DEFAULT NULL COMMENT 'FK to civicrm_contact, who created this contribution' AFTER `token_status`, ADD CONSTRAINT `FK_civicrm_contribution_tappay_created_id` FOREIGN KEY (`created_id`) REFERENCES `civicrm_contact`(`id`) ON DELETE SET NULL ON UPDATE RESTRICT";
    CRM_Core_DAO::executeQuery($sql);
  }
}
