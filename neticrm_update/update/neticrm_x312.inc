<?php
function _neticrm_update_update_x312() {
  $optionGroupId = CRM_Core_DAO::singleValueQuery("SELECT id FROM civicrm_option_group WHERE name LIKE 'pcp_status'");
  if ($optionGroupId) {
    $pcpStatus = CRM_Core_OptionGroup::values('pcp_status',  FALSE, FALSE, FALSE, NULL, 'name');
    $exists = array_search('Draft', $pcpStatus);
    if ($exists === FALSE) {
      $sql = "INSERT INTO `civicrm_option_value`
      (`option_group_id`, `label`, `value`, `name`, `grouping`, `filter`, `is_default`, `weight`, `description`, `is_optgroup`, `is_reserved`, `is_active`, `component_id`, `visibility_id`)
      VALUES
      (%1, %2, 0, 'Draft' , NULL, 0, NULL, 0, NULL, 0, 1, 1, NULL, NULL)";
      CRM_Core_DAO::executeQuery($sql, array(
        1 => array($optionGroupId, 'Integer'),
        2 => array(ts("Draft"), 'String'),
      ));
    }
  }
}

