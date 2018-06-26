<?php
/**
 * @file
 * This file should be civicrm compliant.
 */
function _neticrm_update_update_6136() {
  $sql = "CREATE TABLE IF NOT EXISTS civicrm_sequence (name varchar(64) NOT NULL,  value varchar(255), timestamp double, PRIMARY KEY ( name ))  ENGINE=MEMORY";
  CRM_Core_DAO::executeQuery($sql);
  return 'success create civicrm_sequence table';
}
