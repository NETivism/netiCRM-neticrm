<?php
/**
 * @file
 * Update for 6108
 *
 * This file should be civicrm compliant.
 */
function _neticrm_update_update_6108(){
  $params = array('version' => 3, 'name' => 'recur_frequency_units');
  $result = civicrm_api('OptionGroup', 'Get', $params);
  if(!empty($result['id'])){
    $query_params = array(
      1 => array($result['id'], 'Integer'),
      2 => array('week', 'String'),
      3 => array('day', 'String'),
    );
    CRM_Core_DAO::executeQuery("Update civicrm_option_value SET is_active = 0 WHERE option_group_id = %1 AND ( name = %2 || name = %3)", $query_params);

    // update contribution page settings
    $query = CRM_Core_DAO::executeQuery("SELECT id, recur_frequency_unit FROM civicrm_contribution_page WHERE recur_frequency_unit LIKE '%week%' OR recur_frequency_unit LIKE '%day%'");
    while($query->fetch()){
      $units = explode(CRM_Core_BAO_CustomOption::VALUE_SEPERATOR, $query->recur_frequency_unit);
      foreach($units as $k => $v){
        if($v == 'day' || $v == 'week'){
          unset($units[$k]);
        }
      }
      $units = implode(CRM_Core_BAO_CustomOption::VALUE_SEPERATOR, $units);
      $params = array(
        1 => array($units, 'String'),
        2 => array($query->id, 'Integer'),
      );
      CRM_Core_DAO::executeQuery("UPDATE civicrm_contribution_page SET recur_frequency_unit = %1, is_recur_interval = 0 WHERE id = %2", $params);
    }
    return 'Successful update frequency unit option.';
  }
  return FALSE;
}
