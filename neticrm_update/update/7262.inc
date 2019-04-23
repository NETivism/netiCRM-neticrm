<?php
function _neticrm_update_update_7262(){
  CRM_Core_DAO::executeQuery("UPDATE civicrm_address SET city = '頭份市' WHERE city = '頭份鎮'");
  return "Updated 頭份鎮 to 頭份市";
}