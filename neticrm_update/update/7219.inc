<?php
/**
 * @file
 * This file should be civicrm compliant.
 */
function _neticrm_update_update_7219() {
  $options = [];
  CRM_Core_OptionGroup::getAssoc('msg_tpl_workflow_receipt', $options, TRUE);
  foreach ($options as $wid => $opt) {
    if ($opt['name'] == 'receipt_letter') {
      $workflow_id = $wid;
      break;
    }
  }
  if (!empty($workflow_id)) {
    $params = [
        1 => ['{$sort_name}收', 'String'],
        2 => ['{$addressee}收', 'String'],
        3 => [$workflow_id, 'Integer'],
    ];
    $success1 = CRM_Core_DAO::executeQuery("UPDATE civicrm_msg_template SET msg_html = REPLACE(`msg_html`, %1, %2) WHERE workflow_id = %3", $params);

    $params = [
        1 => ['協會簽章', 'String'],
        2 => ['組織簽章', 'String'],
        3 => [$workflow_id, 'Integer'],
    ];
    $success2 = CRM_Core_DAO::executeQuery("UPDATE civicrm_msg_template SET msg_html = REPLACE(`msg_html`, %1, %2) WHERE workflow_id = %3", $params);
  }
  if ($success1 && $success2) {
    return 'Update receipt word successfuly';
  }
  else {
    throw new DrupalUpdateException('Cannot update string, failed.');
  }
}
