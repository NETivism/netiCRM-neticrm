<?php
/**
 * @file
 * This file should be civicrm compliant.
 */
function _neticrm_update_update_7235() {
  civicrm_initialize();
  $check_query = "SELECT column_name FROM INFORMATION_SCHEMA.COLUMNS WHERE table_name = 'civicrm_event' AND column_name = 'allow_cancel_by_link'";
  $exists = CRM_Core_DAO::singleValueQuery($check_query);
  if(!$exists){
    $alter_query = "ALTER TABLE `civicrm_event` ADD `allow_cancel_by_link` tinyint(4) NULL COMMENT 'If true, allow participant cancel event by link in confirm mail.' AFTER `bcc_confirm`";
    CRM_Core_DAO::executeQuery($alter_query);
  }
  return 'Add allow_cancel_by_link column to civicrm_event success';
}