<?php
/**
 * @file
 * This file should be civicrm compliant.
 */
function _neticrm_update_update_7223() {
  CRM_Core_DAO::executeQuery("UPDATE civicrm_msg_template SET msg_subject = REPLACE(msg_subject, '\n', '') WHERE workflow_id IS NOT NULL AND msg_subject LIKE '%\n'");
  // online contribution and membership
  $params = array(
    1 => array('{if $is_pay_later}{ts}Invoice{/ts} - {elseif $is_monetary}{ts}Payment Notification{/ts} - {/if}{$title}', 'String'),
    2 => array('{if $is_pay_later}{ts}Invoice{/ts}{else}{ts}Payment Notification{/ts}{/if} - {$title}', 'String'),
  );
  $success = CRM_Core_DAO::executeQuery("UPDATE civicrm_msg_template SET msg_subject = %1 WHERE msg_subject LIKE %2", $params);
  
  if ($success) {
    return 'Update receipt word successfuly';
  }
  else {
    throw new DrupalUpdateException('Cannot update string, failed.');
  }
}
