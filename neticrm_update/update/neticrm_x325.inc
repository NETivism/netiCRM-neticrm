<?php

function _neticrm_update_update_x325() {
  $groupName = 'custom_search';
  $allCustom = CRM_Core_OptionGroup::values($groupName, FALSE, FALSE, FALSE, NULL, 'name', FALSE);
  $groupParams = array('name' => $groupName);

  $class = 'CRM_Contact_Form_Search_Custom_RFM';
  $label = ts('RFM Customer Segments');
  $params = [
    'name' => $class,
    'label' => $label,
    'is_active' => 1,
  ];


  if (array_search($class, $allCustom) !== FALSE) {
    $optionValueId = CRM_Core_DAO::singleValueQuery("SELECT id FROM civicrm_option_value WHERE name LIKE %1", array(1 => array($class, 'String')));
    if (!empty($optionValueId)) {
      $action = CRM_Core_Action::UPDATE;
      CRM_Core_OptionValue::addOptionValue($params, $groupParams, $action, $optionValueId);
    } else {
      $action = CRM_Core_Action::ADD;
      $optionValueId = 0;
      CRM_Core_OptionValue::addOptionValue($params, $groupParams, $action, $optionValueId);
    }
  }

  // clear cache
  CRM_Core_OptionGroup::values($groupName, FALSE, FALSE, FALSE, NULL, 'label', TRUE, TRUE);
  return 'Successful update custom search label, add custom search RFM';
}