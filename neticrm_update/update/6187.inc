<?php
function _neticrm_update_update_6187() {
  $dao = CRM_Core_DAO::executeQuery("SELECT provider_id FROM civicrm_im WHERE 1 GROUP BY provider_id");
  $existsProvider = array();
  while($dao->fetch()) {
    $existsProvider[] = $dao->provider_id;
  }
  $disableProvider = array_diff(array(1,2,3,4,5), $existsProvider);
  $optionGroupId = CRM_Core_DAO::singleValueQuery("SELECT id FROM civicrm_option_group WHERE name LIKE 'instant_messenger_service'");
  $sql = "UPDATE civicrm_option_value SET is_active = 0 WHERE option_group_id = %1 AND value IN (".implode(',', $disableProvider).")"; 
  CRM_Core_DAO::executeQuery($sql, array(
    1 => array($optionGroupId, 'Integer'),
  ));
  return "Successfully disable unused IM";
}