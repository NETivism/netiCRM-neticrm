<?php
/**
 * @file
 * Update for 6004
 *
 * This file should be civicrm compliant.
 */
function _neticrm_update_update_6004(){
  module_load_include('inc', 'neticrm_update', 'neticrm_update.lib');
  $to_add = array(
    'CRM_Contact_Form_Search_Custom_PriceSetContribution' => ts('Export Price Set Info for a Contribution Page'),
    'CRM_Contact_Form_Search_Custom_RecurSearch' => ts('Recurring contributions'),
  );
  foreach($to_add as $k => $v){
    neticrm_update_add_custom_search($k, $v);
  }
  return "Successful add new custom search functions.";
}
