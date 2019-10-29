<?php
/**
 * @file
 * This file should be civicrm compliant.
 */
function _neticrm_update_update_7272() {
  $check = "SELECT column_name FROM INFORMATION_SCHEMA.COLUMNS WHERE table_name = 'civicrm_mailing' AND column_name = 'body_preview'";
  $exists = CRM_Core_DAO::singleValueQuery($check);
  if (!$exists) {
    $body_preview = "ALTER TABLE `civicrm_mailing` ADD `body_preview` VARCHAR(128) NULL DEFAULT NULL AFTER `subject`";
    CRM_Core_DAO::executeQuery($body_preview);
  }
  $check = "SELECT column_name FROM INFORMATION_SCHEMA.COLUMNS WHERE table_name = 'civicrm_mailing' AND column_name = 'body_json'";
  $exists = CRM_Core_DAO::singleValueQuery($check);
  if (!$exists) {
    $body_json = "ALTER TABLE `civicrm_mailing` ADD `body_json` LONGTEXT NOT NULL AFTER `body_html`";
    CRM_Core_DAO::executeQuery($body_json);
  }
}
