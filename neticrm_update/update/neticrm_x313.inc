<?php

function _neticrm_update_update_x313() {
  civicrm_initialize();

  $path = drupal_get_path('module', 'civicrm').'/../xml/templates/message_templates/';
  $tpls = array(
    array(
      'msg_title' => '個人募款頁 - 支持者狀態更改通知',
      'msg_text_tpl' => 'pcp_status_change_text.tpl',
      'msg_html_tpl' => 'pcp_status_change_html.tpl',
    ),
    array(
      'msg_title' => '個人募款頁 - 歡迎支持者',
      'msg_text_tpl' => 'pcp_supporter_notify_text.tpl',
      'msg_html_tpl' => 'pcp_supporter_notify_html.tpl',
    ),
    array(
      'msg_title' => '個人募款頁 - 管理通知',
      'msg_text_tpl' => 'pcp_notify_text.tpl',
      'msg_html_tpl' => 'pcp_notify_html.tpl',
    ),
  );

  foreach ($tpls as $tpl) {
    // 取得 text, html tpl 內容
    $tpl_text = file_get_contents($path.'/'.$tpl['msg_text_tpl']);
    $tpl_html = file_get_contents($path.'/'.$tpl['msg_html_tpl']);

    // 取得目前預設的 $msg_workflow_id, $msg_html, $msg_text
    $msg_title = $tpl['msg_title'];
    $params = array(1 => array($msg_title, 'String'));
    $msg_workflow_id = CRM_Core_DAO::singleValueQuery("SELECT workflow_id FROM civicrm_msg_template WHERE msg_title = %1 AND is_reserved = 1", $params);

    print("msg_workflow_id : $msg_workflow_id\n");

    if ($msg_workflow_id) {
      print("有預設的範本");
      $msg_text = CRM_Core_DAO::singleValueQuery("SELECT msg_text FROM civicrm_msg_template WHERE workflow_id = $msg_workflow_id AND is_reserved = 1;");
      $msg_html = CRM_Core_DAO::singleValueQuery("SELECT msg_html FROM civicrm_msg_template WHERE workflow_id = $msg_workflow_id AND is_reserved = 1;");

      // 取得目前的客製範本
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
      if ($is_customized) {
        print("有客製範本");
        print("is_customized : $is_customized\n");
        // $customized_text = CRM_Core_DAO::singleValueQuery("SELECT msg_text FROM civicrm_msg_template WHERE workflow_id = $msg_workflow_id AND is_reserved = 0");
        // $customized_html = CRM_Core_DAO::singleValueQuery("SELECT msg_html FROM civicrm_msg_template WHERE workflow_id = $msg_workflow_id AND is_reserved = 0");
      }

      // 如果 tpl 和預設不一樣，取代預設範本
      if ($msg_html != $tpl_html) {
        print("html tpl 和預設不一樣，取代預設範本\n");
        $params = array(
          1 => array($tpl_html, 'String'),
        );
        CRM_Core_DAO::executeQuery("UPDATE civicrm_msg_template SET msg_html = %1 WHERE workflow_id = $msg_workflow_id AND is_reserved = 1", $params);

        if (empty($diverted_id)) {
          print("沒有客製範本，套用範本到客製範本\n");
          CRM_Core_DAO::executeQuery("UPDATE civicrm_msg_template SET msg_html = %1 WHERE msg_title = '$msg_title' AND is_reserved = 0", $params);
        } else {
          print("有客製範本，不套用到客製範本\n");
        }
      } else {
        print("預設範本已是最新的，不用更新\n");
      }

      if ($msg_text != $tpl_text) {
        print("text tpl 和預設不一樣，取代預設範本\n");
        $params = array(
          1 => array($tpl_text, 'String'),
        );
        CRM_Core_DAO::executeQuery("UPDATE civicrm_msg_template SET msg_text = %1 WHERE workflow_id = $msg_workflow_id AND is_reserved = 1", $params);

        if (empty($diverted_id)) {
          print("沒有客製範本，套用範本到客製範本\n");
          CRM_Core_DAO::executeQuery("UPDATE civicrm_msg_template SET msg_text = %1 WHERE msg_title = '$msg_title' AND is_reserved = 0", $params);
        } else {
          print("有客製範本，不套用到客製範本\n");
        }
      } else {
        print("預設範本已是最新的，不用更新");
      }
    }
  }
}
