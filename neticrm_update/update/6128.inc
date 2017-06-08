<?php
/**
 * @file
 * This file should be civicrm compliant.
 */
function _neticrm_update_update_6128() {
  $sql = "SELECT f.id FROM `civicrm_custom_field` f INNER JOIN civicrm_custom_group g ON f.custom_group_id = g.id WHERE f.name = 'shou_ju_bian_hao' AND g.name LIKE TRIM('shou_ju_zi_xun')";
  $custom_field_id = CRM_Core_DAO::singleValueQuery($sql);
  $a = CRM_Core_DAO::executeQuery("UPDATE civicrm_custom_field SET label = '報稅憑證' WHERE id = %1 AND label = '收據編號'", array(1 => array($custom_field_id, 'Integer')));
  $b = CRM_Core_DAO::executeQuery("UPDATE civicrm_uf_field SET label = '報稅憑證' WHERE field_name = %1 AND label = '收據編號'", array(1 => array('custom_'.$custom_field_id, 'String')));

  // #20424, wrong setting in uf_field
  $uf_group_id = CRM_Core_DAO::singleValueQuery("SELECT uf_group_id FROM civicrm_uf_field WHERE field_name = %1 AND id < 50", array(1 => array('custom_'.$custom_field_id, 'String')));
  if ($uf_group_id) { 
    CRM_Core_DAO::singleValueQuery("UPDATE civicrm_uf_field SET is_searchable = 0, visibility = 'User and User Admin Only' WHERE uf_group_id = %1", array(1 => array($uf_group_id, 'Integer')));
  }
  CRM_Core_DAO::singleValueQuery("UPDATE civicrm_uf_field SET is_searchable = 0, visibility = 'User and User Admin Only' WHERE field_name = 'legal_identifier' AND id < 50");

  if ($a && $b) {
    return 'Successful update receipt field label';
  }
  else {
    return 'No exists receipt field label for update';
  }
}
