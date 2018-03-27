<?php

function _neticrm_update_update_7241(){
  // Refs _neticrm_update_update_7212
  $params = array(
    'label' => ts('CiviMail'),
    'name' => 'CiviMail',
    'description' => '',
    'is_active' => 1,
    'weight' => 20,
  );
  $group_params = array('name' => 'advanced_search_options');
  $action = CRM_Core_Action::ADD;
  $option_name = CRM_Core_DAO::singleValueQuery("SELECT v.name FROM civicrm_option_value v INNER JOIN civicrm_option_group g ON v.option_group_id = g.id WHERE g.name = 'advanced_search_options' AND v.name = 'CiviMail'");
  if(empty($option_name)) {
    $custom_search_id = NULL;
    $custom_search = CRM_Core_OptionValue::addOptionValue($params, $group_params, $action, $custom_search_id);
    if (!empty($custom_search->value)) {
      $option_name = $custom_search->name;
    }
  }

  if($option_name){
    $pref = CRM_Core_BAO_Preferences::valueOptions('advanced_search_options');
    $pref[$option_name] = 1;
    CRM_Core_BAO_Preferences::setValue('advanced_search_options', $pref);    
    return "Successful add advanced search pane of mailing";
  }else{
    return FALSE;
  }

}
