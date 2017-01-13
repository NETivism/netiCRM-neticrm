<?php
/**
 * @file
 * This file should be civicrm compliant.
 */
function _neticrm_update_update_6120() {
  $params = array(
    1 => array('{ts}Contribution Receipt{/ts}', 'String'),
    2 => array('{ts}Payment Notification{/ts}', 'String'),
    3 => array('{ts}Contribution Receipt{/ts}%%', 'String'),
  );
  $success1 = CRM_Core_DAO::executeQuery("UPDATE civicrm_msg_template SET msg_subject = REPLACE(`msg_subject`, %1, %2) WHERE msg_subject LIKE %3", $params);

  $params = array(
    1 => array('{ts}Receipt{/ts}', 'String'),
    2 => array('{ts}Payment Notification{/ts}', 'String'),
    3 => array('{if $is_pay_later}{ts}Invoice{/ts}{else}{ts}Receipt{/ts}{/if}%%', 'String'),
  );
  $success2 = CRM_Core_DAO::executeQuery("UPDATE civicrm_msg_template SET msg_subject = REPLACE(`msg_subject`, %1, %2) WHERE msg_subject LIKE %3", $params);

  $params = array(
    1 => array('Confirmation and Receipt{/ts}', 'String'),
    2 => array('Confirmation{/ts}', 'String'),
    3 => array('{ts}Membership %Confirmation and Receipt{/ts}%%', 'String'),
  );
  $success3 = CRM_Core_DAO::executeQuery("UPDATE civicrm_msg_template SET msg_subject = REPLACE(`msg_subject`, %1, %2) WHERE msg_subject LIKE %3", $params);
  if ($success1 && $success2 && $success3) {
    return 'Update receipt word successfuly';
  }
  else {
    throw new DrupalUpdateException('Cannot update string, failed.');
  }
}
