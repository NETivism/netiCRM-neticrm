<?php
/**
 * @file
 * This file should be civicrm compliant.
 */
function _neticrm_update_update_7275() {
  // first, update batch db
  $dao = CRM_Core_DAO::executeQuery("SHOW COLUMNS FROM civicrm_batch WHERE FIELD like 'status_id'");
  $dao->fetch();
  if (empty($dao->Field)) {
    $sql = "ALTER TABLE `civicrm_batch` ADD `data` LONGTEXT COMMENT 'fk to Batch Status options in civicrm_option_values' AFTER `modified_date`";
    CRM_Core_DAO::executeQuery($sql);
    $sql = "ALTER TABLE `civicrm_batch` ADD `type_id` INT UNSIGNED COMMENT 'fk to Batch Status options in civicrm_option_values' AFTER `modified_date`";
    CRM_Core_DAO::executeQuery($sql);
    $sql = "ALTER TABLE `civicrm_batch` ADD `status_id` INT UNSIGNED NOT NULL COMMENT 'fk to Batch Status options in civicrm_option_values' AFTER `modified_date`";
    CRM_Core_DAO::executeQuery($sql);
  }

  // second, add option group and option values
  $typeGroupId= CRM_Core_DAO::singleValueQuery("SELECT id FROM civicrm_option_group WHERE name LIKE 'batch_type'");
  if (empty($typeGroupId)) {
    CRM_Core_DAO::executeQuery("INSERT INTO civicrm_option_group (`name`, `label`,`description`,`is_reserved`,`is_active`) VALUES ('batch_type', NULL, 'Batch Type', 1, 1)");
    $typeGroupId = CRM_Core_DAO::singleValueQuery("SELECT id FROM civicrm_option_group WHERE name LIKE 'batch_type'");
    // values
    CRM_Core_DAO::executeQuery("INSERT INTO civicrm_option_value 
    (`option_group_id`, `label`, `value`, `name`, `grouping`, `filter`, `is_default`, `weight`, `description`, `is_optgroup`, `is_reserved`, `is_active`, `component_id`, `domain_id`, `visibility_id`) 
    VALUES ($typeGroupId, '".ts('Auto')."', 1, 'Auto', NULL, 0, 1, 1, NULL, 0, 1, 1, NULL, 1, NULL)");
  }
  $statusGroupId = CRM_Core_DAO::singleValueQuery("SELECT id FROM civicrm_option_group WHERE name LIKE 'batch_status'");
  if (empty($statusGroupId)) {
    CRM_Core_DAO::executeQuery("INSERT INTO civicrm_option_group (`name`, `label`,`description`,`is_reserved`,`is_active`) VALUES ('batch_status', NULL, 'Batch Status', 1, 1)");
    $statusGroupId = CRM_Core_DAO::singleValueQuery("SELECT id FROM civicrm_option_group WHERE name LIKE 'batch_status'");
    CRM_Core_DAO::executeQuery("INSERT INTO civicrm_option_value 
    (`option_group_id`, `label`, `value`, `name`, `grouping`, `filter`, `is_default`, `weight`, `description`, `is_optgroup`, `is_reserved`, `is_active`, `component_id`, `domain_id`, `visibility_id`) 
    VALUES 
    ($statusGroupId, '".ts('Completed')."', 1, 'Completed', NULL, 0, 0, 1, NULL, 0, 1, 1, NULL, 1, NULL),
    ($statusGroupId, '".ts('Pending')."', 2, 'Pending', NULL, 0, 0, 2, NULL, 0, 1, 1, NULL, 1, NULL),
    ($statusGroupId, '".ts('Running')."', 3, 'Running', NULL, 0, 0, 3, NULL, 0, 1, 1, NULL, 1, NULL),
    ($statusGroupId, '".ts("Canceled")."', 4, 'Canceled', NULL, 0, 0, 4, NULL, 0, 1, 1, NULL, 1, NULL) 
    ");
  }

  // third, add message template to exists workflow
  $metaWorkflowId = CRM_Core_DAO::singleValueQuery("SELECT id FROM civicrm_option_group WHERE name = 'msg_tpl_workflow_meta'");
  if(!empty($metaWorkflowId)){
    $exists = CRM_Core_DAO::singleValueQuery("SELECT id FROM civicrm_option_value WHERE name = 'batch_complete_notification'");
    if (!$exists) {
      $group_params = array(
        'id' => $metaWorkflowId,
      );
      $option_params = array(
        'label' => ts('Batch Complete Notification'),
        'name' => 'batch_complete_notification',
        'is_active' => TRUE,
      );
      $action = CRM_Core_Action::ADD;
      $option_id = 0;
      $option = CRM_Core_OptionValue::addOptionValue($option_params, $group_params, $action, $option_id);
      $subject = file_get_contents(drupal_get_path('module', 'civicrm').'/../xml/templates/message_templates/batch_complete_notification_subject.tpl');
      $text = file_get_contents(drupal_get_path('module', 'civicrm').'/../xml/templates/message_templates/batch_complete_notification_text.tpl');
      $html = file_get_contents(drupal_get_path('module', 'civicrm').'/../xml/templates/message_templates/batch_complete_notification_html.tpl');

      $params = array(
        'msg_title' => ts('Batch Complete Notification'),
        'msg_subject' => $subject,
        'msg_text' => $text,
        'msg_html' => $html,
        'is_active' => TRUE,
        'workflow_id' => $option->id,
        'is_default' => 1,
        'is_reserved' => 0,
      );
      // add default template
      CRM_Core_BAO_MessageTemplates::add($params);

      // add reserved template for recover
      $params['is_default'] = 0;
      $params['is_reserved'] = 1;
      CRM_Core_BAO_MessageTemplates::add($params);
    }
  }
}
