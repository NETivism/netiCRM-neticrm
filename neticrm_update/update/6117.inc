<?php
/**
 * @file
 * This file should be civicrm compliant.
 */
function _neticrm_update_update_6117() {
  $check_query = "SELECT column_name FROM INFORMATION_SCHEMA.COLUMNS WHERE table_name = 'civicrm_contribution_type' AND column_name = 'tax_rate'";
  $exists = CRM_Core_DAO::singleValueQuery($check_query);
  if(!$exists){
    $alter_query = "ALTER TABLE `civicrm_contribution_type` ADD `tax_rate` tinyint UNSIGNED DEFAULT 0 AFTER description";
    CRM_Core_DAO::executeQuery($alter_query);
    return "Successful add tax_rate, is_taxreceipt column";
  }

  return "Column exists, doesn't need update";
}

