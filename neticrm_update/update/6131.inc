<?php
/**
 * @file
 * This file should be civicrm compliant.
 */
function _neticrm_update_update_6131() {
  $groupName = 'custom_search';

  // move label to name, description to label, empty description
  $allCustom = CRM_Core_OptionGroup::values($groupName, FALSE, FALSE, FALSE, NULL, 'name', FALSE);
  foreach ($allCustom as $value => $name) {
    if (strstr($name, 'CRM_Contact_Form_Search_Custom')) {
      $id = CRM_Core_DAO::singleValueQuery("SELECT id FROM civicrm_option_value WHERE label = %1", array(1=> array($name, 'String')));
      if (trim($id)) {
        $sql = "UPDATE civicrm_option_value SET label = description, description = '' WHERE id = $id";
        CRM_Core_DAO::executeQuery($sql);
      }
    }
  }

  $groupParams = array('name' => $groupName);

  $searchs = array(
    'FirstTimeDonor' => ts('First time donation donors'),
    'HalfYearDonor' => ts('Donor who donate in last %count month', array('count' => 6, 'plural' => 'Donor who donate in last %count months')),
    'ContribSYBNT' => ts('Last year but not this year donors'),
    'FailedNoFurtherDonate' => ts('After payment failed but not retry in %1 days', array(1 => '7')),
    'SingleNotRecurring' => ts('Single donation over %1 times', array(1 => '3')),
    'AttendeeNotDonor' => ts('Attendee but not donor'),
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
