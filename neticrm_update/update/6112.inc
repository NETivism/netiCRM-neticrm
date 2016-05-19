<?php
/**
 * @file
 *
 * This file should be civicrm compliant.
 */
function _neticrm_update_update_6112(){
  $params = array(
    'label' => ts('CiviMail'),
    'name' => 'CiviMail',
    'description' => '',
    'is_active' => 1,
    'weight' => 20,
  );
  $group_params = array('name' => 'advanced_search_options');
  $action = CRM_Core_Action::ADD;
  $option_id = CRM_Core_DAO::singleValueQuery("SELECT v.id FROM civicrm_option_value v INNER JOIN civicrm_option_group g ON v.option_group_id = g.id WHERE g.name = 'advanced_search_options' AND v.name = 'CiviMail'");
  if(empty($option_id)) {
    $custom_search_id = NULL;
    $custom_search = CRM_Core_OptionValue::addOptionValue($params, $group_params, $action, $custom_search_id);
    if (!empty($custom_search->value)) {
      $pref = CRM_Core_BAO_Preferences::valueOptions('advanced_search_options');
      $pref[$custom_search->name] = 1;
      CRM_Core_BAO_Preferences::setValue('advanced_search_options', $pref);
    }
  }
  return "Successful add advanced search pane of mailing";
}
