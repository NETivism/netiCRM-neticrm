<?php

function _neticrm_update_update_x313() {
  $tpls = [
    [
      'workflow' => 'msg_tpl_workflow_contribution',
      'message' => 'pcp_status_change',
    ],
    [
      'workflow' => 'msg_tpl_workflow_contribution',
      'message' => 'pcp_supporter_notify',
    ],
    [
      'workflow' => 'msg_tpl_workflow_contribution',
      'message' => 'pcp_notify',
    ],
  ];

  foreach ($tpls as $tpl) {
    _civicrm_demo_create_update_msg_tpl($tpl['workflow'], $tpl['message']);
  }
}
