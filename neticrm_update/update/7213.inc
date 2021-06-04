<?php
/**
 * @file
 * Update for 7206
 *
 * This file should be civicrm compliant.
 */
function _neticrm_update_update_7213(){
  civicrm_initialize();
  $workflow = CRM_Core_DAO::singleValueQuery("SELECT id FROM civicrm_option_value WHERE name = 'receipt_letter'");

  // Checked if the msg is customized
  $sql = '
          SELECT diverted.id
          FROM civicrm_msg_template diverted JOIN civicrm_msg_template orig ON (
              diverted.workflow_id = orig.workflow_id AND
              orig.workflow_id = %1                   AND
              orig.is_reserved = 1                    AND
                  diverted.msg_html    != orig.msg_html
          )
      ';
  $params = array(1 => array($workflow, 'Integer'));
  $diverted_id = CRM_Core_DAO::singleValueQuery($sql,$params);
  $is_customized = !empty($diverted_id);
  if($is_customized){
    $customized_html = CRM_Core_DAO::singleValueQuery("SELECT msg_html FROM civicrm_msg_template WHERE workflow_id = $workflow AND is_reserved = 0");
  }

  // Remove origin msg template
  $id = CRM_Core_DAO::singleValueQuery("SELECT id FROM civicrm_msg_template WHERE workflow_id = $workflow AND is_reserved = 1");
  if($id){
    CRM_Core_BAO_MessageTemplates::del($id);
  }
  $id = CRM_Core_DAO::singleValueQuery("SELECT id FROM civicrm_msg_template WHERE workflow_id = $workflow AND is_reserved = 0");
  if($id){
    CRM_Core_BAO_MessageTemplates::del($id);
  }

  // Add new msg template
  $name = '紙本收據';
  $path = drupal_get_path('module', 'civicrm').'/../xml/templates/message_templates/receipt_letter_';
  if(is_file($path.'html.zh_TW.tpl')){
    $html = $is_customized ? $customized_html : file_get_contents($path.'html.zh_TW.tpl');
    
    $text = file_get_contents($path.'text.tpl');
    $subject = file_get_contents($path.'subject.tpl');
    $params = array(
      'msg_title' => $name,
      'msg_subject' => $subject,
      'msg_text' => $text,
      'msg_html' => $html,
      'is_active' => TRUE,
      'workflow_id' => $workflow,
      'is_default' => 1,
      'is_reserved' => 0,
    );
    // add default template
    CRM_Core_BAO_MessageTemplates::add($params);
    $params['is_default'] = 0;
    $params['is_reserved'] = 1;
    if($is_customized){
      $params['msg_html'] = file_get_contents($path.'html.zh_TW.tpl');
    }

    // add reserved template
    CRM_Core_BAO_MessageTemplates::add($params);

    return "Success to update receipt_letter message template.";
  }else{
    return array('#abort' => array('success' => FALSE, 'query' => 'You need /xml/templates/message_templates/receipt_letter_html.zh_TW.tpl in CiviCrm direction to finish update.'));
  }
}