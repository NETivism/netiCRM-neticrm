<?php
/**
 * @file
 * This file should be civicrm compliant.
 */
function _neticrm_update_update_7220() {
  $params = [
    1 => ['{ts}Contribution Receipt{/ts}', 'String'],
    2 => ['{ts}Payment Notification{/ts}', 'String'],
    3 => ['{ts}Contribution Receipt{/ts}%%', 'String'],
  ];
  $success1 = CRM_Core_DAO::executeQuery("UPDATE civicrm_msg_template SET msg_subject = REPLACE(`msg_subject`, %1, %2) WHERE msg_subject LIKE %3", $params);

  $params = [
    1 => ['{ts}Receipt{/ts}', 'String'],
    2 => ['{ts}Payment Notification{/ts}', 'String'],
    3 => ['{if $is_pay_later}{ts}Invoice{/ts}{else}{ts}Receipt{/ts}{/if}%%', 'String'],
  ];
  $success2 = CRM_Core_DAO::executeQuery("UPDATE civicrm_msg_template SET msg_subject = REPLACE(`msg_subject`, %1, %2) WHERE msg_subject LIKE %3", $params);

  $params = [
    1 => ['Confirmation and Receipt{/ts}', 'String'],
    2 => ['Confirmation{/ts}', 'String'],
    3 => ['{ts}Membership %Confirmation and Receipt{/ts}%%', 'String'],
  ];
  $success3 = CRM_Core_DAO::executeQuery("UPDATE civicrm_msg_template SET msg_subject = REPLACE(`msg_subject`, %1, %2) WHERE msg_subject LIKE %3", $params);
  if ($success1 && $success2 && $success3) {
    return 'Update receipt word successfuly';
  }
  else {
    throw new DrupalUpdateException('Cannot update string, failed.');
  }
}
