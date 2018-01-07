<?php
function _neticrm_update_update_7239(){
  $msg_title = ts('Events - Registration Confirmation Invite');
  $params = array(1 => array($msg_title, 'String'));
  $msg_id = CRM_Core_DAO::singleValueQuery("SELECT id FROM civicrm_msg_template WHERE msg_title = %1 AND is_reserved = 1", $params);

  if(!empty($msg_id)){
  }

  // updating template
  $path = drupal_get_path('module', 'civicrm').'/../xml/templates/message_templates/participant_confirm_';
  if(!empty($msg_id) && is_file($path.'html.tpl')){
    $html = file_get_contents($path.'html.tpl');
    $text = file_get_contents($path.'text.tpl');
    $params = array(
      1 => array($html, 'String'),
      2 => array($text, 'String'),
      3 => array($msg_id, 'Integer'),
    );
    CRM_Core_DAO::executeQuery("UPDATE civicrm_msg_template SET msg_html = %1, msg_text = %2 WHERE id = $msg_id", $params);
    return "Successful update message templates of online contribution. (#22026)";
  }
  return FALSE;
}
