<?php
function _neticrm_update_update_6186() {
  CRM_Core_DAO::executeQuery("UPDATE civicrm_preferences_date SET start = 30 WHERE start = 20");
  return "Successfully update date range.";
}