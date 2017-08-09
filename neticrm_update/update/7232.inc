<?php
/**
 * @file
 * This file should be civicrm compliant.
 * Refs 7227.inc
 */
function _neticrm_update_update_7232() {
  civicrm_initialize();

  // 取得正確的 tpl html 
  $path = drupal_get_path('module', 'civicrm').'/../xml/templates/message_templates/event_online_receipt_html.tpl';
  $tpl_html = file_get_contents($path);

  // 取得目前預設的 $msg_workflow_id, $msg_html
  $msg_title = '活動 - 登記確認與收據（在線）';
  $params = array(1 => array($msg_title, 'String'));
  $msg_workflow_id = CRM_Core_DAO::singleValueQuery("SELECT workflow_id FROM civicrm_msg_template WHERE msg_title = %1 AND is_reserved = 1", $params);

  print("msg_workflow_id : $msg_workflow_id\n");

  if($msg_workflow_id){
    print("有紙本收據範本");
    $msg_html = CRM_Core_DAO::singleValueQuery("SELECT msg_html FROM civicrm_msg_template WHERE workflow_id = $msg_workflow_id AND is_reserved = 1;");

    // 取得目前客製的範本
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
    $params = array(1 => array($msg_workflow_id, 'Integer'));
    $diverted_id = CRM_Core_DAO::singleValueQuery($sql,$params);
    print("diverted_id : $diverted_id\n");
    $is_customized = !empty($diverted_id);
    if($is_customized){
      print("有客製範本");
      print("is_customized : $is_customized\n");
      $customized_html = CRM_Core_DAO::singleValueQuery("SELECT msg_html FROM civicrm_msg_template WHERE workflow_id = $msg_workflow_id AND is_reserved = 0");
    }

    // 如果 tpl 和預設不一樣，取代預設範本
    if($msg_html != $tpl_html){
      print(" tpl 和預設不一樣，取代預設範本");
      $params = array(
        1 => array($tpl_html, 'String'),
      );
      CRM_Core_DAO::executeQuery("UPDATE civicrm_msg_template SET msg_html = %1 WHERE workflow_id = $msg_workflow_id AND is_reserved = 1", $params);

      if(empty($diverted_id)){
        print("沒有客製收據，套用範本到客製收據");
        CRM_Core_DAO::executeQuery("UPDATE civicrm_msg_template SET msg_html = %1 WHERE msg_title = '$msg_title' AND is_reserved = 0", $params);
      }else{
        print("有客製收據，範本不套用到客製收據");
      }
    }else{
      print("預設範本已是最新的，不用更新");
    }
  }

}