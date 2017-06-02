<?php
/**
 * @file
 * This file should be civicrm compliant.
 */
function _neticrm_update_update_7230() {
  // purge duplicate configs
  _neticrm_update_update_7230_helper('addressee');
  _neticrm_update_update_7230_helper('postal_greeting');
  _neticrm_update_update_7230_helper('email_greeting');
}

function _neticrm_update_update_7230_helper($name) {
  $sql = "SELECT id, label, value FROM civicrm_option_value WHERE option_group_id IN (SELECT id FROM `civicrm_option_group` WHERE name = %1) ORDER BY value, id ASC";
  $dao = CRM_Core_DAO::executeQuery($sql, array(1 => array($name, 'String')));
  $result = array();
  while ($dao->fetch()) {
    $result[$dao->value][] = $dao->id;
  }
  foreach($result as $r) {
    $id = 0;
    if (count($r) > 1) {
      $id = reset($r);
      CRM_Core_DAO::executeQuery("DELETE FROM civicrm_option_value WHERE id = %1", array(1 => array($id, 'Integer')));
    } 
  }
}
