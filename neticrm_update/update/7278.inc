<?php
function _neticrm_update_update_7278() {
  $optionGroupId = CRM_Core_DAO::singleValueQuery("SELECT id FROM civicrm_option_group WHERE name = 'activity_type'");

  $exists = CRM_Core_DAO::singleValueQuery("SELECT name FROM civicrm_option_value WHERE name = 'Email Receipt' AND option_group_id = %1", array(1 => array($optionGroupId, 'Positive')));
  if (!$exists) {
    $maxValue = CRM_Core_DAO::singleValueQuery("SELECT MAX(CAST(value AS UNSIGNED)) FROM civicrm_option_value WHERE option_group_id = %1", array(1 => array($optionGroupId, 'Positive')));
    $componentId = CRM_Core_DAO::singleValueQuery("SELECT MAX(id) FROM civicrm_component WHERE name = 'CiviContribute'");
    $sql = "INSERT INTO `civicrm_option_value` (`option_group_id`, `label`, `value`, `name`, `grouping`, `filter`, `is_default`, `weight`, `description`, `is_optgroup`, `is_reserved`, `is_active`, `component_id`, `visibility_id`) VALUES (%1, '{Email Receipt}', %2, 'Email Receipt', NULL, 1, 0, %2, '', 0, 1, 0, %3, NULL);";
    $sql = str_replace("{Email Receipt}", ts("Email Receipt"), $sql);
    $params = array(
      1 => array($optionGroupId, 'Positive'),
      2 => array($maxValue + 1, 'Positive'),
      3 => array($componentId, 'Positive'),
    );
    CRM_Core_DAO::executeQuery($sql, $params);
  }
  $exists = CRM_Core_DAO::singleValueQuery("SELECT name FROM civicrm_option_value WHERE name = 'Print Contribution Receipts' AND option_group_id = %1", array(1 => array($optionGroupId, 'Positive')));
  if (!$exists) {
    $maxValue = CRM_Core_DAO::singleValueQuery("SELECT MAX(CAST(value AS UNSIGNED)) FROM civicrm_option_value WHERE option_group_id = %1", array(1 => array($optionGroupId, 'Positive')));
    $componentId = CRM_Core_DAO::singleValueQuery("SELECT MAX(id) FROM civicrm_component WHERE name = 'CiviContribute'");
    $sql = "INSERT INTO `civicrm_option_value` (`option_group_id`, `label`, `value`, `name`, `grouping`, `filter`, `is_default`, `weight`, `description`, `is_optgroup`, `is_reserved`, `is_active`, `component_id`, `visibility_id`) VALUES (%1, '{Print Contribution Receipts}', %2, 'Print Contribution Receipts', NULL, 1, 0, %2, '', 0, 1, 1, %3, NULL);";
    $sql = str_replace("{Print Contribution Receipts}", ts("Print Contribution Receipts"), $sql);
    $params = array(
      1 => array($optionGroupId, 'Positive'),
      2 => array($maxValue + 1, 'Positive'),
      3 => array($componentId, 'Positive'),
    );
    CRM_Core_DAO::executeQuery($sql, $params);
  }
  echo "Print Contribution Receipts and Email Receipt added to activity types.";

  // Contribution SMS and Event Registration SMS
  $exists = CRM_Core_DAO::singleValueQuery("SELECT name FROM civicrm_option_value WHERE name = 'Contribution SMS' AND option_group_id = %1", array(1 => array($optionGroupId, 'Positive')));
  if (!$exists) {
    $maxValue = CRM_Core_DAO::singleValueQuery("SELECT MAX(CAST(value AS UNSIGNED)) FROM civicrm_option_value WHERE option_group_id = %1", array(1 => array($optionGroupId, 'Positive')));
    $componentId = CRM_Core_DAO::singleValueQuery("SELECT MAX(id) FROM civicrm_component WHERE name = 'CiviContribute'");
    $sql = "INSERT INTO `civicrm_option_value` (`option_group_id`, `label`, `value`, `name`, `grouping`, `filter`, `is_default`, `weight`, `description`, `is_optgroup`, `is_reserved`, `is_active`, `component_id`, `visibility_id`) VALUES (%1, '{Contribution SMS}', %2, 'Contribution SMS', NULL, 1, 0, %2, '', 0, 1, 1, %3, NULL);";
    $sql = str_replace("{Contribution SMS}", ts("Contribution SMS"), $sql);
    $params = array(
      1 => array($optionGroupId, 'Positive'),
      2 => array($maxValue + 1, 'Positive'),
      3 => array($componentId, 'Positive'),
    );
    CRM_Core_DAO::executeQuery($sql, $params);
  }

  $exists = CRM_Core_DAO::singleValueQuery("SELECT name FROM civicrm_option_value WHERE name = 'Event Registration SMS' AND option_group_id = %1", array(1 => array($optionGroupId, 'Positive')));
  if (!$exists) {
    $maxValue = CRM_Core_DAO::singleValueQuery("SELECT MAX(CAST(value AS UNSIGNED)) FROM civicrm_option_value WHERE option_group_id = %1", array(1 => array($optionGroupId, 'Positive')));
    $componentId = CRM_Core_DAO::singleValueQuery("SELECT MAX(id) FROM civicrm_component WHERE name = 'CiviEvent'");
    $sql = "INSERT INTO `civicrm_option_value` (`option_group_id`, `label`, `value`, `name`, `grouping`, `filter`, `is_default`, `weight`, `description`, `is_optgroup`, `is_reserved`, `is_active`, `component_id`, `visibility_id`) VALUES (%1, '{Event Registration SMS}', %2, 'Event Registration SMS', NULL, 1, 0, %2, '', 0, 1, 1, %3, NULL);";
    $sql = str_replace("{Event Registration SMS}", ts("Event Registration SMS"), $sql);
    $params = array(
      1 => array($optionGroupId, 'Positive'),
      2 => array($maxValue + 1, 'Positive'),
      3 => array($componentId, 'Positive'),
    );
    CRM_Core_DAO::executeQuery($sql, $params);
  }
  echo "Contribution SMS and Event Registration SMS added to activity types.";
}

