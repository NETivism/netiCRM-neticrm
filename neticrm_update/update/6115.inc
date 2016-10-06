<?php
/**
 * @file
 *
 * This file should be civicrm compliant.
 */
function _neticrm_update_update_6115(){
  CRM_Core_DAO::executeQuery("UPDATE civicrm_address SET city = '臺東市' WHERE city = '台東市'");
  return "Updated 台東市 to 臺東市";
}

