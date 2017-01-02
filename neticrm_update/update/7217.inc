<?php
/**
 * @file
 * This file should be civicrm compliant.
 */
function _neticrm_update_update_7217() {
  $check_query = "SELECT column_name FROM INFORMATION_SCHEMA.COLUMNS WHERE table_name = 'civicrm_contribution_type' AND column_name = 'is_taxreceipt'";
  $exists = CRM_Core_DAO::singleValueQuery($check_query);
  if(!$exists){
    $alter_query = "ALTER TABLE `civicrm_contribution_type` ADD `is_taxreceipt` tinyint UNSIGNED DEFAULT 0 AFTER description";
    CRM_Core_DAO::executeQuery($alter_query);
  }

  $check_query = "SELECT column_name FROM INFORMATION_SCHEMA.COLUMNS WHERE table_name = 'civicrm_contribution_type' AND column_name = 'tax_rate'";
  $exists = CRM_Core_DAO::singleValueQuery($check_query);
  if(!$exists){
    $alter_query = "ALTER TABLE `civicrm_contribution_type` ADD `tax_rate` tinyint UNSIGNED DEFAULT 0 AFTER description";
    CRM_Core_DAO::executeQuery($alter_query);
  }

  return "Alter field successful";
}
