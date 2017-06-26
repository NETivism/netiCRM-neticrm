<?php
/**
 * @file
 * This file should be civicrm compliant.
 */
function _neticrm_update_update_6131() {
  $allCustom = CRM_Core_OptionGroup::values('custom_search', FALSE, FALSE, FALSE, NULL, 'name');
  foreach ($allCustom as $value => $name) {
    if (strstr($name, 'CRM_Contact_Form_Search_Custom')) {
      $id = CRM_Core_DAO::singleValueQuery("SELECT id FROM civicrm_option_value WHERE label = %1", array(1=> array($name, 'String')));
      if (trim($id)) {
        $sql = "UPDATE civicrm_option_value SET label = description, description = '' WHERE id = $id";
        CRM_Core_DAO::executeQuery($sql);
      }
    }
  }
  return 'Successful update custom search label';
}
