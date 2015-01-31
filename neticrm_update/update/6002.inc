<?php
/**
 * @file
 * Update for 6002
 *
 * This file should be civicrm compliant.
 */
function _neticrm_update_update_6002(){
  CRM_Core_DAO::executeQuery("UPDATE civicrm_state_province SET name = 'Taoyuan City' WHERE id = 4862");
  $query = CRM_Core_DAO::executeQuery("SELECT id, city FROM civicrm_address WHERE state_province_id = 4862"); 
  while($query->fetch()){
    $city = preg_replace("/(鄉|鎮|市)$/ui", "區", $query->city);
    $sql = "UPDATE civicrm_address SET city = %1 WHERE id = %2";
    $params = array(
      1 => array($city, 'String'),
      2 => array($query->id, 'Integer'),
    );
    CRM_Core_DAO::executeQuery($sql, $params);
  }
  return "Successful update Taoyuan address";
}
