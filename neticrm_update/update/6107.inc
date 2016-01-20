<?php
/**
 * @file
 * Update for 6107
 *
 * This file should be civicrm compliant.
 */
function _neticrm_update_update_6107(){
  $title = '臺灣財政部所得扣除額';
  $desc = '提供給財政部所得扣除額的報表';
  $url = 'contribute/taiwantax';
  $class= 'CRM_Report_Form_Contribute_TaiwanTax';

  $components = CRM_Core_Component::getComponents();
  $result = neticrm_update_add_report($title, $desc, $url, $class, $components['CiviContribute']->componentID);
  if($result->id){
    return 'Successful added report';
  }
  else{
    return FALSE;
  }
}
