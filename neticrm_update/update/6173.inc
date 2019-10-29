<?php
/**
 * @file
 * This file should be civicrm compliant.
 */
function _neticrm_update_update_6172() {
  $check = "SELECT column_name FROM INFORMATION_SCHEMA.COLUMNS WHERE table_name = 'civicrm_product' AND column_name = 'calculate_mode'";
  $exists = CRM_Core_DAO::singleValueQuery($check);
  if (!$exists) {
    CRM_Core_DAO::executeQuery("ALTER TABLE `civicrm_product` ADD `calculate_mode` VARCHAR(32) DEFAULT NULL COMMENT 'Calculate by accumulate or by min-amount of contribution' AFTER `currency`");
    CRM_Core_DAO::executeQuery("ALTER TABLE `civicrm_product` ADD `installments` INT DEFAULT '0' COMMENT 'Num of installments when calculate mode is accumulate.' AFTER `calculate_mode`");
    CRM_Core_DAO::executeQuery("ALTER TABLE `civicrm_product` ADD `min_contribution_recur` DECIMAL(20,2) COMMENT 'Minimum recurring contribution amount required to be eligible to select this premium.' AFTER `min_contribution`");
  }
  $check = "SELECT column_name FROM INFORMATION_SCHEMA.COLUMNS WHERE table_name = 'civicrm_premiums' AND column_name = 'premiums_nothanks_text'";
  $exists = CRM_Core_DAO::singleValueQuery($check);
  if (!$exists) {
    CRM_Core_DAO::executeQuery("ALTER TABLE `civicrm_premiums` ADD `premiums_nothanks_text` VARCHAR(128) NULL COMMENT 'No thank you text for premium page display.' AFTER `premiums_display_min_contribution`");
  }
}