<?php
/**
 * @file
 * Update for 6000
 *
 * This file should be civicrm compliant.
 */
function _neticrm_update_update_6000(){
  $vard_names = array(
    'HOME' => 'Home',
    'WORK' => 'Work',
  );
  $descriptions = array(
    '主要辦公地點' => 'Primary',
    '其他地點' => 'Other',
  );
  $q = CRM_Core_DAO::executeQuery("SHOW columns from `civicrm_location_type` where field='label'");
  if($q->N === 0){
    CRM_Core_DAO::executeQuery("ALTER TABLE `civicrm_location_type` ADD `label` VARCHAR(255) NOT NULL AFTER `id`, ADD INDEX (`label`) COMMENT 'Location type display label'");
    $q = CRM_Core_DAO::executeQuery("SELECT * FROM `civicrm_location_type` WHERE label = ''");
    while($q->fetch()){
      $new_name = !empty($vard_names[$q->vcard_name]) ? $vard_names[$q->vcard_name] : (!empty($descriptions[$q->description]) ? $descriptions[$q->description] : '');
      if(empty($new_name) && $q->name == '帳單'){
        $new_name = 'Billing';
      }
      if(!empty($new_name)){
        CRM_Core_DAO::executeQuery("UPDATE `civicrm_location_type` set label = '{$q->name}', name = '$new_name' WHERE id = $q->id");
      }
      else{
        CRM_Core_DAO::executeQuery("UPDATE `civicrm_location_type` set label = '{$q->name}' WHERE id = $q->id");
      }
    }
  }
  return "Successful update civicrm location type";
}
