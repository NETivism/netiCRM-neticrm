<?php
function _neticrm_update_update_x333() {
  $exists = CRM_Core_DAO::singleValueQuery("
    SELECT COLUMN_NAME
    FROM INFORMATION_SCHEMA.COLUMNS
    WHERE TABLE_SCHEMA = DATABASE()
      AND TABLE_NAME = 'civicrm_contribution_mypay_log'
      AND COLUMN_NAME = 'uid'
  ");
  if (!$exists) {
    CRM_Core_DAO::executeQuery("
      ALTER TABLE civicrm_contribution_mypay_log
        ADD COLUMN uid varchar(255) DEFAULT NULL COMMENT 'The uid in mypay transaction.' AFTER contribution_id,
        ADD INDEX UI_uid(uid)
    ");
    echo "civicrm_contribution_mypay_log.uid added successfully.\n";
  }
  else {
    echo "civicrm_contribution_mypay_log.uid already exists.\n";
  }
}
