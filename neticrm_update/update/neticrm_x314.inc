<?php

function _neticrm_update_update_x314() {
  civicrm_initialize();

  // Add 'is_sync' column to 'civicrm_group' table if it doesn't exist
  $query = "SHOW COLUMNS FROM `civicrm_group` LIKE 'is_sync'";
  $result = CRM_Core_DAO::executeQuery($query);
  if ($result->N == 0) {
    $sql = "ALTER TABLE `civicrm_group` ADD `is_sync` tinyint COMMENT 'Indicates whether the group is synchronized.'";
    CRM_Core_DAO::executeQuery($sql);
  }

  // Add 'sync_data' column to 'civicrm_group' table if it doesn't exist
  $query = "SHOW COLUMNS FROM `civicrm_group` LIKE 'sync_data'";
  $result = CRM_Core_DAO::executeQuery($query);
  if ($result->N == 0) {
    $sql = "ALTER TABLE `civicrm_group` ADD `sync_data` text COMMENT 'Stores JSON format data from the remote group, such as remote group ID, name, creation time.'";
    CRM_Core_DAO::executeQuery($sql);
  }

  // Add 'last_sync' column to 'civicrm_group' table if it doesn't exist
  $query = "SHOW COLUMNS FROM `civicrm_group` LIKE 'last_sync'";
  $result = CRM_Core_DAO::executeQuery($query);
  if ($result->N == 0) {
    $sql = "ALTER TABLE `civicrm_group` ADD `last_sync` datetime COMMENT 'Stores the last sync time with the remote group.'";
    CRM_Core_DAO::executeQuery($sql);
  }

  // Since `sms_provider_name` option group was already checked in `_neticrm_update_update_7238`, retrieve its ID directly here
  $query = "SELECT id FROM civicrm_option_group WHERE name = 'sms_provider_name'";
  $sms_provider_name_group_id = CRM_Core_DAO::singleValueQuery($query);

  // Check if a SMS provider named 'Flydove' already exists
  $check_query = "SELECT count(*) FROM civicrm_option_value WHERE option_group_id = $sms_provider_name_group_id AND name = 'CRM_SMS_Provider_Flydove'";
  $exists = CRM_Core_DAO::singleValueQuery($check_query);

  // If 'Flydove' SMS provider does not exist, insert it as a new option
  if (!$exists) {
    $group_params = array('name' => 'sms_provider_name');
    $action = CRM_Core_Action::ADD;
    $option_value_id = 0;
    $params = array(
      'option_group_id' => $sms_provider_name_group_id,
      'label' => 'Flydove',
      'value' => 'CRM_SMS_Provider_Flydove',
      'weight' => 2,
      'is_reserved' => 1,
      'is_active' => 1,
    );

    CRM_Core_OptionValue::addOptionValue($params, $group_params, $action, $option_value_id);

    // Clear cache
    CRM_Core_OptionGroup::values('sms_provider_name', FALSE, FALSE, FALSE, NULL, 'label', TRUE, TRUE);
  }
}