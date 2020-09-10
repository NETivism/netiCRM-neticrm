<?php
function _neticrm_update_update_7287() {
  $dao = CRM_Core_DAO::executeQuery("SELECT provider_id FROM civicrm_im WHERE 1 GROUP BY provider_id");
  $existsProvider = array();
  while($dao->fetch()) {
    $existsProvider[] = $dao->provider_id;
  }
  $disableProvider = array_diff(array(1,2,3,4,5), $existsProvider);
  $optionGroupId = CRM_Core_DAO::singleValueQuery("SELECT id FROM civicrm_option_group WHERE name LIKE 'instant_messenger_service'");
  $sql = "UPDATE civicrm_option_value SET is_active = 0 WHERE option_group_id = %1 AND value IN (".implode(',', $disableProvider).")";
  $params = array(
    1 => array($optionGroupId, 'Integer'),
  );
  CRM_Core_DAO::executeQuery($sql, $params);

  $insertValues = array('LINE', 'Facebook');
  foreach ($insertValues as $imName) {
    $params[2] = array( $imName, 'String');
    $exist = CRM_Core_DAO::singleValueQuery("SELECT name FROM civicrm_option_value WHERE name = %2 AND option_group_id = %1", $params);
    if (empty($exist)) {
      $max = CRM_Core_DAO::singleValueQuery("SELECT MAX(CAST(value AS UNSIGNED)) FROM civicrm_option_value WHERE option_group_id = %1", $params);
      $params[3] = array( $max+1, 'Positive');
      $sql = "INSERT INTO `civicrm_option_value` (`option_group_id`, `label`, `value`, `name`, `grouping`, `filter`, `is_default`, `weight`, `description`, `is_optgroup`, `is_reserved`, `is_active`, `component_id`, `visibility_id`) VALUES
       (%1, %2,  %3, %2,  NULL, 0, NULL, %3, NULL, 0, 0, 1, NULL, NULL)";
       CRM_Core_DAO::executeQuery($sql, $params);
    }
  }

  return "Successfully disable unused IM, update new IM";
}