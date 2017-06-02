<?php
/**
 * @file
 * This file should be civicrm compliant.
 */
function _neticrm_update_update_6129() {
  CRM_Core_DAO::executeQuery("UPDATE civicrm_address SET city = '員林市' WHERE city = '員林鎮'");
  return "Updated 員林鎮 to 員林市";
}
