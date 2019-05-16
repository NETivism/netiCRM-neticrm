<?php
function _neticrm_update_update_6164(){
  $word = ts('Suspended');
  $option_group_id_cs = CRM_Core_DAO::singleValueQuery("SELECT id FROM civicrm_option_group WHERE name = 'contribution_status'");
  CRM_Core_DAO::executeQuery("INSERT IGNORE INTO civicrm_option_value (`option_group_id`, `label`, `value`, `name`, `grouping`, `filter`, `is_default`, `weight`, `description`, `is_optgroup`, `is_reserved`, `is_active`, `component_id`, `visibility_id`) SELECT $option_group_id_cs, '$word'  , 7, 'Suspended'  , NULL, 0, NULL, 7, NULL, 0, 1, 1, NULL, NULL FROM civicrm_option_value WHERE NOT EXISTS ( SELECT name FROM civicrm_option_value WHERE `option_group_id` = $option_group_id_cs AND `name` = 'Suspended' ) LIMIT 1");
  return "Add suspended to contribution_status";
}
