<?php
function _neticrm_update_update_6169(){
  // add is_qrcode column to event
  $sql = "SHOW COLUMNS FROM civicrm_event WHERE field LIKE 'is_qrcode'";
  $dao = CRM_Core_DAO::executeQuery($sql);
  if (empty($dao->N)) {
    $sql = "ALTER TABLE `civicrm_event` ADD `is_qrcode` tinyint UNSIGNED DEFAULT 0 COMMENT 'If true, include qrcode image on event registered email notification.' AFTER is_online_registration ";
    $dao = CRM_Core_DAO::executeQuery($sql);
  }

  // update default msg template
  $msg_id = CRM_Core_DAO::singleValueQuery("SELECT m.id FROM `civicrm_msg_template` m INNER JOIN civicrm_option_value v ON v.id = m.workflow_id WHERE v.name = 'event_online_receipt' AND m.is_reserved = 1", $params);
  $path = drupal_get_path('module', 'civicrm').'/../xml/templates/message_templates/';
  if (!empty($msg_id) && is_file($path.'event_online_receipt_html.tpl')) {
    $html = file_get_contents($path.'event_online_receipt_html.tpl');
    $text = file_get_contents($path.'event_online_receipt_text.tpl');
    $params = array(
      1 => array($html, 'String'),
      2 => array($text, 'String'),
      3 => array($msg_id, 'Integer'),
    );
    CRM_Core_DAO::executeQuery("UPDATE civicrm_msg_template SET msg_html = %1, msg_text = %2 WHERE id = $msg_id", $params);
  }
  unset($msg_id);
  $msg_id = CRM_Core_DAO::singleValueQuery("SELECT m.id FROM `civicrm_msg_template` m INNER JOIN civicrm_option_value v ON v.id = m.workflow_id WHERE v.name = 'event_offline_receipt' AND m.is_reserved = 1", $params);
  $path = drupal_get_path('module', 'civicrm').'/../xml/templates/message_templates/';
  if (!empty($msg_id) && is_file($path.'event_offline_receipt_html.tpl')) {
    $html = file_get_contents($path.'event_offline_receipt_html.tpl');
    $text = file_get_contents($path.'event_offline_receipt_text.tpl');
    $params = array(
      1 => array($html, 'String'),
      2 => array($text, 'String'),
      3 => array($msg_id, 'Integer'),
    );
    CRM_Core_DAO::executeQuery("UPDATE civicrm_msg_template SET msg_html = %1, msg_text = %2 WHERE id = $msg_id", $params);
  }
}
