<?php
/**
 * Example function to doing database update
 *
 * @return mixed Success: NULL, TRUE, String. Failed: FALSE or string contain "[error]"
 */
function _neticrm_update_update_x304() {
  $dao = CRM_Core_DAO::executeQuery("SELECT data_type FROM information_schema.columns WHERE `table_name` = 'civicrm_contribution_taiwanach' AND column_name = 'order_number'");
  $dao->fetch();
  if ($dao->data_type !== 'varchar' && empty($dao->data_type)) {
    $sql = "ALTER TABLE `civicrm_contribution_taiwanach` ADD `order_number` VARCHAR(128) NULL DEFAULT NULL COMMENT 'Order number of ACH Account' AFTER `identifier_number`";
    CRM_Core_DAO::executeQuery($sql);
    return 'Added field civicrm_contribution_taiwanach.order';
  }
  else {
    return 'Already exists, skipped.';
  }
}