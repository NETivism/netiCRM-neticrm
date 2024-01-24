<?php

function _neticrm_update_update_x313() {
  civicrm_initialize();

  $path = __DIR__ . '/../../../xml/templates/message_templates/';
  $tpls = array(
    array(
      'workflow' => 'msg_tpl_workflow_contribution',
      'message' => 'pcp_status_change',
    ),
    array(
      'workflow' => 'msg_tpl_workflow_contribution',
      'message' => 'pcp_supporter_notify',
    ),
    array(
      'workflow' => 'msg_tpl_workflow_contribution',
      'message' => 'pcp_notify',
    ),
  );

  foreach ($tpls as $tpl) {
    _civicrm_demo_create_update_msg_tpl($tpl['workflow'], $tpl['message']);
  }
}