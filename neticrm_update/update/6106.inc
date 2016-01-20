<?php
/**
 * @file
 * Update for 6106
 *
 * This file should be civicrm compliant.
 */
function _neticrm_update_update_6106(){
  // see if exists group
  $exists = CRM_Core_DAO::singleValueQuery("SELECT id FROM civicrm_option_value WHERE name = 'contribution_invoice_notify'");
  $name = ts('Contributions - Invoice Notification');
  if(empty($exists)){
    $gid = CRM_Core_DAO::singleValueQuery("SELECT id FROM civicrm_option_group WHERE name = 'msg_tpl_workflow_contribution'");
    $group_params = array(
      'name' => 'msg_tpl_workflow_contribution',
      'is_active' => 1,
      'is_reserved' => 0,
    );
    $option_params = array(
      'label' => $name,
      'name' => 'contribution_invoice_notify',
      'is_active' => 1,
    );
    $action = CRM_Core_Action::ADD;
    $option_id = 0;
    $option = CRM_Core_OptionValue::addOptionValue($option_params, $group_params, $action, $option_id);

    $path = drupal_get_path('module', 'civicrm').'/../xml/templates/message_templates/contribution_invoice_notify_';
    if(is_file($path.'html.tpl')){
      $html = file_get_contents($path.'html.tpl');
      $text = file_get_contents($path.'text.tpl');
      $subject = file_get_contents($path.'subject.tpl');
      $params = array(
        'msg_title' => $name,
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
      $params['is_default'] = 0;
      $params['is_reserved'] = 1;

      // add reserved template
      CRM_Core_BAO_MessageTemplates::add($params);
    }
  }
  return FALSE;
}
