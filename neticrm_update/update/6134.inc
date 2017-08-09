<?php
/**
 * @file
 * This file should be civicrm compliant.
 */
function _neticrm_update_update_6134() {
  $sql = "ALTER TABLE `civicrm_mailing_trackable_url` CHANGE `url` `url` VARCHAR(511) NOT NULL COMMENT 'The URL to be tracked.'";
  $dao = CRM_Core_DAO::executeQuery($sql);
  return 'Successful extend the length of url field';
}
