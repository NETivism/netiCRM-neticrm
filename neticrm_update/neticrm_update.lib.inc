<?php

/**
 * Add custom search if name not exists.
 *
 * @param $name string
 * @param $desc string
 * return integer id of custom search
 */
function neticrm_update_add_custom_search($name, $desc){
  $find = array(
    1 => array($name, 'String')
  );
  $exists = CRM_Core_DAO::singleValueQuery("SELECT value FROM civicrm_option_value WHERE name LIKE %1", $find);

  if(!$exists){
    $params = array(
      'label' => $name,
      'name' => $name,
      'description' => ts($desc),
      'is_active' => 1,
    );
    $group_params = array('name' => 'custom_search');
    $action = CRM_Core_Action::ADD;
    $custom_search_id = NULL;
    $custom_search = CRM_Core_OptionValue::addOptionValue($params, $group_params, $action, $custom_search_id);
    return $custom_search->value;
  }
  else{
    return $exists;
  }
}

/**
 * Add new civicrm report template and instance
 * 
 * @param $label string
 * @param $desc string
 * @param $url  string
 * @param $class string
 * @param $component_id integer 
 * return integer instance_id
 */
function neticrm_update_add_report($label, $desc, $url, $class, $component_id){
  $group_params = array('name' => ('report_template'));
  require_once 'CRM/Core/OptionValue.php';

  $exists = CRM_Core_OptionGroup::getRowValues('report_template', $class, 'name', 'String', FALSE);
  if(!empty($exists)){
    $action = CRM_Core_Action::UPDATE;
    $id = $exists['id'];
  }
  else{
    $action = CRM_Core_Action::ADD;
    $id = NULL;
  }
  
  $params = array(
    'label' => $label,
    'description' => $desc,
    'value' => $url,
    'name' => $class,
    'component_id' => $component_id,
    'is_active' => 1,
    'weight' => 50,
  );
  $added = CRM_Core_OptionValue::addOptionValue($params, $group_params, $action, $id);
  return $added;
}
