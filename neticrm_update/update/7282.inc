<?php

function _neticrm_update_update_7282() {
  $search = [
    '{if $is_pay_later}{ts}Invoice{/ts} - {elseif $is_monetary}{ts}Payment Notification{/ts} - {/if}{$title}',
    '{ts}Payment Notification{/ts}',
  ];
  $path = drupal_get_path('module', 'civicrm').'/../xml/templates/message_templates/';
  $replace = [
    trim(file_get_contents($path."contribution_online_receipt_subject.tpl")),
    trim(file_get_contents($path."contribution_offline_receipt_subject.tpl")),
  ];
  foreach ($search as $k => $pattern) {
    CRM_Core_DAO::executeQuery("UPDATE civicrm_msg_template SET msg_subject = REPLACE(msg_subject, %1, %2) WHERE msg_subject LIKE %1", [
      1 => [$pattern, 'String'],
      2 => [$replace[$k], 'String'],
    ]);
  }
}
