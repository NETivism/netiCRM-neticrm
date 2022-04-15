<?php

/**
 * Example function to doing database update
 *
 * @return mixed Success: NULL, TRUE, String. Failed: FALSE or string contain "[error]"
 */
function _neticrm_update_update_x305() {
  $dao = CRM_Core_DAO::executeQuery("SELECT data_type FROM information_schema.columns WHERE `table_name` = 'civicrm_mailing' AND column_name = 'is_hidden'");
  $dao->fetch();
  if ($dao->data_type !== 'tinyint' && empty($dao->data_type)) {
    $sql = "ALTER TABLE `civicrm_mailing` ADD `is_hidden` TINYINT DEFAULT '0' COMMENT 'Is this mailing hidden?' AFTER `sms_provider_id`";
    CRM_Core_DAO::executeQuery($sql);
  }

  // add activity types 
  _neticrm_update_add_activity_type_x304('Contribution Notification Email', 'CiviContribute');
  _neticrm_update_add_activity_type_x304('Event Notification Email', 'CiviEvent');
  _neticrm_update_add_activity_type_x304('Membership Notification Email', 'CiviMember');
  _neticrm_update_add_activity_type_x304('PCP Notification Email', '');
}

function _neticrm_update_add_activity_type_x304($type, $component) {
  $optionGroupId = CRM_Core_DAO::singleValueQuery("SELECT id FROM civicrm_option_group WHERE name = 'activity_type'");

  $exists = CRM_Core_DAO::singleValueQuery("SELECT name FROM civicrm_option_value WHERE name = %1 AND option_group_id = %2", array(
    1 => array($type, 'String'),
    2 => array($optionGroupId, 'Positive')
  ));
  if (!$exists) {
    $maxValue = CRM_Core_DAO::singleValueQuery("SELECT MAX(CAST(value AS UNSIGNED)) FROM civicrm_option_value WHERE option_group_id = %1", array(1 => array($optionGroupId, 'Positive')));
    $params = array(
      1 => array($optionGroupId, 'Positive'),
      2 => array($maxValue + 1, 'Positive'),
      3 => array(ts($type), 'String'),
      4 => array($type, 'String'),
    );
    if (!empty($component)) {
      $componentId = CRM_Core_DAO::singleValueQuery("SELECT MAX(id) FROM civicrm_component WHERE name = %1", array(1 => array($component, 'String')));
      $params[5] = array($componentId, 'Positive');
      $sql = "INSERT INTO `civicrm_option_value` (`option_group_id`, `label`, `value`, `name`, `grouping`, `filter`, `is_default`, `weight`, `description`, `is_optgroup`, `is_reserved`, `is_active`, `component_id`, `visibility_id`) VALUES (%1, %3, %2, %4, NULL, 1, 0, %2, '', 0, 1, 1, %5, NULL);";
    }
    else {
      $sql = "INSERT INTO `civicrm_option_value` (`option_group_id`, `label`, `value`, `name`, `grouping`, `filter`, `is_default`, `weight`, `description`, `is_optgroup`, `is_reserved`, `is_active`, `component_id`, `visibility_id`) VALUES (%1, %3, %2, %4, NULL, 1, 0, %2, '', 0, 1, 1, NULL, NULL);";
    }
    CRM_Core_DAO::executeQuery($sql, $params);
  }
}
