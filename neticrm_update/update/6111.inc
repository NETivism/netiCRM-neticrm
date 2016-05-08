<?php
/**
 * @file
 * Update for 6111
 *
 * This file should be civicrm compliant.
 */
function _neticrm_update_update_6111(){
  $check_query = "SELECT column_name FROM INFORMATION_SCHEMA.COLUMNS WHERE table_name = 'civicrm_price_field_value' AND column_name = 'is_member'";
  $exists = CRM_Core_DAO::singleValueQuery($check_query);

  if(!$exists){
    $alter_query = "ALTER TABLE `civicrm_price_field_value` ADD `is_member` tinyint DEFAULT 0 AFTER weight";
    CRM_Core_DAO::executeQuery($alter_query);
    return "Successful add is_member column";
  }

  return "Column exists, doesn't need update";
}
