<?php
/**
 * @file
 * This file should be civicrm compliant.
 */
function _neticrm_update_update_7221() {
  civicrm_initialize();
  $check_query = "SELECT column_name FROM INFORMATION_SCHEMA.COLUMNS WHERE table_name = 'civicrm_contribution_page' AND column_name = 'recur_fail_notify'";
  $exists = CRM_Core_DAO::singleValueQuery($check_query);
  if(!$exists){
    $alter_query = "ALTER TABLE `civicrm_contribution_page` ADD `recur_fail_notify` VARCHAR(255) NULL  COMMENT 'comma-separated list of email addresses to each time a recurring is failed' AFTER `receipt_text`";
    CRM_Core_DAO::executeQuery($alter_query);
  }


  // Check: 
  $id = CRM_Core_DAO::singleValueQuery("SELECT ov.id FROM civicrm_option_value ov INNER JOIN civicrm_option_group og WHERE og.name = 'msg_tpl_workflow_contribution' AND ov.name = 'contribution_recur_fail_notify'");
  if(!empty($id)){
    return FALSE;
  }
  $gid = CRM_Core_DAO::singleValueQuery("SELECT id FROM civicrm_option_group WHERE name = 'msg_tpl_workflow_contribution'");
  if(empty($gid)){
    return FALSE;
  }

  $group_params = array(
    'id' => $gid,
  );
  $option_params = array(
    'label' => ts('Contributions - Recurring Failed Notification'),
    'name' => 'contribution_recur_fail_notify',
    'is_active' => TRUE,
  );
  $option_id = NULL;
  $action = CRM_Core_Action::ADD;
  
  $option = CRM_Core_OptionValue::addOptionValue($option_params, $group_params, $action, $option_id);

  $path = drupal_get_path('module', 'civicrm').'/../xml/templates/message_templates/contribution_recur_fail_notify_subject.tpl';
  $subject = file_get_contents($path);
  $path = drupal_get_path('module', 'civicrm').'/../xml/templates/message_templates/contribution_recur_fail_notify_text.tpl';
  $text = file_get_contents($path);
  $path = drupal_get_path('module', 'civicrm').'/../xml/templates/message_templates/contribution_recur_fail_notify_html.tpl';
  $html = file_get_contents($path);
  $params = array(
    'msg_title' => ts('Contributions - Recurring Failed Notification'),
    'msg_subject' => $subject,
    'msg_text' => $text,
    'msg_html' => $html,
    'is_active' => TRUE,
    'workflow_id' => $option->id,
    'is_default' => 1,
    'is_reserved' => 0,
  );
  // add default template
  $default_tpl = CRM_Core_BAO_MessageTemplates::add($params);
  $params['is_default'] = 0;
  $params['is_reserved'] = 1;
  $reserved_tpl = CRM_Core_BAO_MessageTemplates::add($params);

}
