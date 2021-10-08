<?php
/**
 * Example function to doing database update
 *
 * @return mixed Success: NULL, TRUE, String. Failed: FALSE or string contain "[error]"
 */
function _neticrm_update_update_x302() {
  $dao = CRM_Core_DAO::executeQuery("SHOW INDEX FROM civicrm_contribution WHERE Key_name LIKE 'ML_contribution'");
  $dao->fetch();
  if ($dao->N <= 0) {
    $sql = "ALTER TABLE civicrm_contribution ADD INDEX `ML_contribution` (`contact_id`, `contribution_type_id`, `contribution_page_id`, `payment_processor_id`, `payment_instrument_id`, `created_date`, `receive_date`, `total_amount`, `trxn_id`, `receipt_date`, `contribution_recur_id`, `is_test`, `contribution_status_id`, `receipt_id`)";
    CRM_Core_DAO::executeQuery($sql);
    return 'Index ML_contribution created.';
  }
  else {
    return 'Index exists, skipped.';
  }
}
