<?php

/**
 * Example function to doing database update
 *
 * @return mixed Success: NULL, TRUE, String. Failed: FALSE or string contain "[error]"
 */
function _neticrm_update_update_x310() {
  $dao = CRM_Core_DAO::executeQuery("SHOW INDEX FROM civicrm_mailing_trackable_url WHERE Key_name LIKE 'index_trackable_url'");

  $dao->fetch();
  if ($dao->N <= 0) {
    $sql = "ALTER TABLE `civicrm_mailing_trackable_url` ADD INDEX `index_trackable_url` (`id`)";
    CRM_Core_DAO::executeQuery($sql);
    return 'Index index_trackable_url created.';
  }
  return 'Index exists, skipped.';
}
