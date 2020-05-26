<?php

function _neticrm_update_update_7279() {
  $groupName = 'custom_search';
  $allCustom = CRM_Core_OptionGroup::values($groupName, FALSE, FALSE, FALSE, NULL, 'name', FALSE);
  $groupParams = array('name' => $groupName);

  $searchs = array(
    'UpcomingBirthdays' => ts('Upcoming Birthdays Search'),
    'RecurDonor' => ts('Recurring Donors Search'),
    'TaiwanACHSearch' => ts('ACH Search'),
  );

  foreach($searchs as $name => $label){
    $class = 'CRM_Contact_Form_Search_Custom_'.$name;
    if ($v = array_search($class, $allCustom)) {
      $action = CRM_Core_Action::UPDATE;
      $optionValueId = CRM_Core_DAO::singleValueQuery("SELECT id FROM civicrm_option_value WHERE name LIKE %1", array(1 => array($class, 'String')));
      if (empty($optionValueId)) {
        continue;
      }
    }
    else {
      $action = CRM_Core_Action::ADD;
      $optionValueId = 0;
    }
    $params = array(
      'name' => $class,
      'label' => $label,
      'is_active' => 1,
    );
    CRM_Core_OptionValue::addOptionValue($params, $groupParams, $action, $optionValueId);
  }

  // clear cache
  CRM_Core_OptionGroup::values($groupName, FALSE, FALSE, FALSE, NULL, 'label', TRUE, TRUE);
  return 'Successful update custom search label, add custom search booster';
}