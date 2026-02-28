<?php
function _neticrm_update_update_x317() {
  // add qpdf options
  civicrm_initialize();
  $params = [
    'qpdfPath' => '/usr/bin/qpdf',
  ];
  CRM_Core_BAO_ConfigSetting::add($params);

  // update templates
  $tpls = [
    [
      'workflow' => 'msg_tpl_workflow_contribution',
      'message' => 'contribution_online_receipt',
    ],
    [
      'workflow' => 'msg_tpl_workflow_contribution',
      'message' => 'contribution_offline_receipt',
    ],
  ];

  foreach ($tpls as $tpl) {
    _civicrm_demo_create_update_msg_tpl($tpl['workflow'], $tpl['message']);
  }
}
