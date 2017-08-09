<?php
/**
 * @file
 * This file should be civicrm compliant.
 */
function _neticrm_update_update_7233() {
  $sql = "SELECT contact_id FROM civicrm_address GROUP BY contact_id HAVING SUM(is_billing) < 1";
  $dao = CRM_Core_DAO::executeQuery($sql);
  while ($dao->fetch()) {
    CRM_Core_BAO_Location::checkBillingAddress($dao->contact_id);
  }

  return 'Successful re-roll contact billing address flag';
}
