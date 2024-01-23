<?php

/**
 * Update Alishan township district
 *
 */
function _neticrm_update_update_x312() {
  $q = CRM_Core_DAO::executeQuery("UPDATE civicrm_address SET city='阿里山鄉' WHERE city='阿里山'");
  return "Successful update for Alishan township district";
}
