<?php

/**
 * @file
 * This file should be civicrm compliant.
 */
function _neticrm_update_update_6171()
{
// add is_qrcode column to event
  $sql = "SHOW COLUMNS FROM civicrm_contribution_page WHERE field LIKE 'installments_option'";
  $dao = CRM_Core_DAO::executeQuery($sql);
  if (empty($dao->N)) {
    $sql = "ALTER TABLE `civicrm_contribution_page` ADD installments_option text COMMENT 'Use options for donor to select recurring installments.' AFTER `currency` ";
    $dao = CRM_Core_DAO::executeQuery($sql);
    $sql = "UPDATE `civicrm_contribution_page` SET installments_option = '1' WHERE is_recur = 1";
    $dao = CRM_Core_DAO::executeQuery($sql);
  }
}