<?php
/**
 * @file
 * This file should be civicrm compliant.
 */
function _neticrm_update_update_6116() {
  $sql = "DELETE FROM `civicrm_state_province` WHERE name = 'Taiwan' AND abbreviation = '71'";
  CRM_Core_DAO::executeQuery($sql);
  return "Successful remove Taiwan from China";
}

