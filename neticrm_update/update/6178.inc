<?php
function _neticrm_update_update_6178() {
  $optionGroupId = CRM_Core_DAO::singleValueQuery("SELECT id FROM civicrm_option_group WHERE name = 'activity_type'");
  $exists = CRM_Core_DAO::singleValueQuery("SELECT name FROM civicrm_option_value WHERE name = 'Email Receipt' AND option_group_id = %1", array(1 => array($optionGroupId, 'Positive')));
  if (!$exists) {
    $maxValue = CRM_Core_DAO::singleValueQuery("SELECT MAX(CAST(value AS UNSIGNED)) FROM civicrm_option_value WHERE option_group_id = %1", array(1 => array($optionGroupId, 'Positive')));
    $componentId = CRM_Core_DAO::singleValueQuery("SELECT MAX(id) FROM civicrm_component WHERE name = 'CiviContribute'");
    $sql = "INSERT INTO `civicrm_option_value` (`option_group_id`, `label`, `value`, `name`, `grouping`, `filter`, `is_default`, `weight`, `description`, `is_optgroup`, `is_reserved`, `is_active`, `component_id`, `visibility_id`) VALUES (%1, '{Email Receipt}', %2, 'Email Receipt', NULL,0, 0, %2, '', 0, 1, 1, %3, NULL);";
    $sql = str_replace("{Email Receipt}", ts("Email Receipt"), $sql);
    $params = array(
      1 => array($optionGroupId, 'Positive'),
      2 => array($maxValue + 1, 'Positive'),
      3 => array($componentId, 'Positive'),
    );
    CRM_Core_DAO::executeQuery($sql, $params);
  }
  else {
    echo "Email Receipt is exist in civicrm_option_value.";
  }
}

