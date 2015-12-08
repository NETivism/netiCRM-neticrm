<?php
/**
 * @file
 * Update for 6105
 *
 * This file should be civicrm compliant.
 */
function _neticrm_update_update_6105(){
  $group_params = array('name' => 'safe_file_extension');

  $allowed_extension = array(
    'pptx',
    'odt',
    'ods',
    'odp',
    'zip',
    'rar',
    '7z',
  );
  foreach($allowed_extension as $ext){
    $action = CRM_Core_Action::ADD;
    $option_value_id = 0;
    $params = array(
      'name' => '',
      'label' => $ext,
      'is_active' => 1,
    );
    CRM_Core_OptionValue::addOptionValue($params, $group_params, $action, $option_value_id);
  }
  // clear cache
  CRM_Core_OptionGroup::values('safe_file_extension', FALSE, FALSE, FALSE, NULL, 'label', TRUE, TRUE);
  return "Updated safe file extension. (#16538)";
}
