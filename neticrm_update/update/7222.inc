<?php
/**
 * @file
 * This file should be civicrm compliant.
 */
function _neticrm_update_update_7222() {
  civicrm_initialize();

  // Check data_type 
  $sql = "SELECT `data_type` FROM `civicrm_custom_field` WHERE `civicrm_custom_field`.`name` = 'shi_fou_xu_yao_shou_ju';";
  $data_type = CRM_Core_DAO::singleValueQuery($sql);
  if($data_type = 'String'){
    return false;
  }

  $sql = "UPDATE `civicrm_custom_field` SET `data_type` = 'String' WHERE `civicrm_custom_field`.`name` = 'shi_fou_xu_yao_shou_ju';";
  CRM_Core_DAO::executeQuery($sql);

  $sql = "SELECT MAX(id) FROM civicrm_option_group;";
  $og_max_id = CRM_Core_DAO::singleValueQuery($sql);
  $new_og_id = $og_max_id+1;

  $sql = "UPDATE `civicrm_custom_field` SET `option_group_id` = '$new_og_id' WHERE `civicrm_custom_field`.`name` = 'shi_fou_xu_yao_shou_ju';";
  CRM_Core_DAO::executeQuery($sql);

  $time = date("YmdHis");
  $sql = "INSERT INTO `civicrm_option_group` (`id`, `name`, `label`, `description`, `is_reserved`, `is_active`) VALUES (NULL,'shi_fou_xu_yao_shou_ju_$time', '是否需要收據', '是否需要收據', NULL, '1');";
  CRM_Core_DAO::executeQuery($sql);

  $sql = "INSERT INTO `civicrm_option_value` (`id`, `option_group_id`, `label`, `value`, `name`, `grouping`, `filter`, `is_default`,`weight`, `description`, `is_optgroup`, `is_reserved`, `is_active`, `component_id`, `domain_id`, `visibility_id`) VALUES (NULL,'$new_og_id', '不要（無抵稅需求/不用寄給我收據）', '0', 'no', NULL, NULL, '0', '0', NULL, '0', '0', '1', NULL, NULL, NULL), (NULL, '$new_og_id', '需要（請寄給我紙本收據（年寄））', '1', 'yes', NULL, NULL, '0', '1', NULL, '0', '0', '1', NULL, NULL, NULL), (NULL, '$new_og_id', '需要（不用寄給我/請幫我在隔年五月上傳至國稅局）', '2', 'yes_upload', NULL, NULL, '0', '2', NULL, '0', '0', '0', NULL, NULL, NULL);";
  CRM_Core_DAO::executeQuery($sql);
}
