<?php
/**
 * @file
 * This file should be civicrm compliant.
 */
function _neticrm_update_update_7237() {
  civicrm_initialize();
  $check_query = "SELECT CHARACTER_MAXIMUM_LENGTH FROM INFORMATION_SCHEMA.COLUMNS WHERE TABLE_NAME='civicrm_contact' AND COLUMN_NAME='external_identifier';";
  $length = CRM_Core_DAO::singleValueQuery($check_query);
  if(!$length < 128){
    $alter_query = "ALTER TABLE civicrm_contact MODIFY external_identifier varchar(128);";
    CRM_Core_DAO::executeQuery($alter_query);
    return 'Modify external_identifier to varchar(128)';
  }

  return 'External_identifier is varchar(128) now, doesn`t need to modify.';
}