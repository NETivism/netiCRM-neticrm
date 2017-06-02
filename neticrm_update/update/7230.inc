<?php
/**
 * @file
 * This file should be civicrm compliant.
 */
function _neticrm_update_update_7230() {
  // correct previous wrong setting
  $sql = "UPDATE civicrm_option_value v SET v.label = REPLACE(v.label, 'Dear', '親愛的') WHERE v.option_group_id IN (SELECT id FROM `civicrm_option_group` WHERE name = 'postal_greeting' OR name = 'email_greeting') AND v.label LIKE 'Dear%'";
  CRM_Core_DAO::executeQuery($sql);

  $sql = "UPDATE civicrm_option_value SET label = '{contact.last_name}{contact.first_name}{ }{contact.individual_prefix}{ }{contact.individual_suffix}', name = '{contact.last_name}{contact.first_name}{ }{contact.individual_prefix}{ }{contact.individual_suffix}' WHERE `label` LIKE '{contact.individual_prefix}{ } {contact.first_name}{ }{contact.middle_name}{ }{contact.last_name}{ }{contact.individual_suffix}'";
  CRM_Core_DAO::executeQuery($sql);

  // purge duplicate configs
  _neticrm_update_update_7230_helper('addressee');
  _neticrm_update_update_7230_helper('postal_greeting');
  _neticrm_update_update_7230_helper('email_greeting');
}

function _neticrm_update_update_7230_helper($name) {
  $sql = "SELECT id, label, value FROM civicrm_option_value WHERE option_group_id IN (SELECT id FROM `civicrm_option_group` WHERE name = 'addressee') ORDER BY value ASC";
  $dao = CRM_Core_DAO::executeQuery($sql);
  $result = array();
  while ($dao->fetch()) {
    $result[$dao->label][] = $dao->id;
  }
  foreach($result as $r) {
    $id = 0;
    if (count($r) > 1) {
      $id = reset($r);
      CRM_Core_DAO::executeQuery("DELETE FROM civicrm_option_value WHERE id = %1", array(1 => array($id, 'Integer')));
    } 
  }
}
