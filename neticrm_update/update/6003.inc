<?php
/**
 * @file
 * Update for 6003
 *
 * This file should be civicrm compliant.
 */
function _neticrm_update_update_6003(){
  $msg_title = ts('Contributions - Receipt (on-line)');
  $params = array(1 => array($msg_title, 'String'));
  $msg_id = CRM_Core_DAO::singleValueQuery("SELECT id FROM civicrm_msg_template WHERE title = %1 AND is_default = 1", $params);

  if(empty($msg_id)){
    $msg_title = 'Contributions - Receipt (on-line)';
    $params = array(1 => array($msg_title, 'String'));
    $msg_id = CRM_Core_DAO::singleValueQuery("SELECT id FROM civicrm_msg_template WHERE title = %1 AND is_default = 1", $params);
  }

  // updating template
  $path = drupal_get_path('module', 'civicrm').'/../xml/templates/message_templates/contribution_online_receipt_';
  if(!empty($msg_id) && is_file($path.'html.tpl')){
    $html = file_get_contents($path.'html.tpl');
    $text = file_get_contents($path.'text.tpl');
    $params = array(
      1 => array($html, 'String'),
      2 => array($text, 'String'),
      3 => array($msg_id, 'Integer'),
    );
    CRM_Core_DAO::executeQuery("UPDATE civicrm_msg_template SET html = %1, text = %2 WHERE id = $msg_id", $params);
    return "Successful update message templates of online contribution. (#12866)";
  }
  return FALSE;
}
