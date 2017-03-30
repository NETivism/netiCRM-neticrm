<?php
/**
 * @file
 * This file should be civicrm compliant.
 */
function _neticrm_update_update_7225() {
  // move receive date to cancel date when recurring contribution is failed state
  $sql = "ALTER TABLE `civicrm_contribution_page` CHANGE `is_recur` `is_recur` INT(10) UNSIGNED NULL DEFAULT '0' COMMENT '0 - disabled reucrring, 1 - enable both, 2 - enabled recur only'";
  CRM_Core_DAO::executeQuery($sql);

  return 'Successful update civicrm_contribution_page is_recur field';
}
