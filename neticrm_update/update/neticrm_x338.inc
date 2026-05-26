<?php

function _neticrm_update_update_x338() {
  civicrm_initialize();

  $messages = [];

  // contribution_recur_id column + FK
  $colCheck = CRM_Core_DAO::executeQuery("SHOW COLUMNS FROM `civicrm_contribution_linepay` LIKE 'contribution_recur_id'");
  if ($colCheck->N === 0) {
    $sql = <<<EOT
      ALTER TABLE `civicrm_contribution_linepay`
      ADD COLUMN `contribution_recur_id` int unsigned DEFAULT NULL COMMENT 'FK to civicrm_contribution_recur. Binds the issued preapproved payment regKey to a recurring contribution.' AFTER `id`
    EOT;
    CRM_Core_DAO::executeQuery($sql);
    $messages[] = 'Added contribution_recur_id to civicrm_contribution_linepay.';
  }
  else {
    $messages[] = 'civicrm_contribution_linepay.contribution_recur_id already exists, skipping column.';
  }

  $fkCheck = CRM_Core_DAO::singleValueQuery(
    "SELECT COUNT(*) FROM information_schema.TABLE_CONSTRAINTS
     WHERE TABLE_SCHEMA = DATABASE()
       AND TABLE_NAME = 'civicrm_contribution_linepay'
       AND CONSTRAINT_NAME = 'FK_civicrm_contribution_linepay_contribution_recur_id'
       AND CONSTRAINT_TYPE = 'FOREIGN KEY'"
  );
  if (empty($fkCheck)) {
    $sql = <<<EOT
      ALTER TABLE `civicrm_contribution_linepay`
      ADD CONSTRAINT `FK_civicrm_contribution_linepay_contribution_recur_id`
      FOREIGN KEY (`contribution_recur_id`) REFERENCES `civicrm_contribution_recur`(`id`) ON DELETE SET NULL
    EOT;
    CRM_Core_DAO::executeQuery($sql);
    $messages[] = 'Added FK_civicrm_contribution_linepay_contribution_recur_id.';
  }
  else {
    $messages[] = 'FK_civicrm_contribution_linepay_contribution_recur_id already exists, skipping FK.';
  }

  // reg_key column + index
  $colCheck = CRM_Core_DAO::executeQuery("SHOW COLUMNS FROM `civicrm_contribution_linepay` LIKE 'reg_key'");
  if ($colCheck->N === 0) {
    $sql = <<<EOT
      ALTER TABLE `civicrm_contribution_linepay`
      ADD COLUMN `reg_key` varchar(255) DEFAULT NULL COMMENT 'regKey returned from /payments/{transactionId}/confirm when payType is PREAPPROVED. Used by /payments/preapprovedPay/{regKey}/* APIs for recurring charges.' AFTER `void`
    EOT;
    CRM_Core_DAO::executeQuery($sql);
    $messages[] = 'Added reg_key to civicrm_contribution_linepay.';
  }
  else {
    $messages[] = 'civicrm_contribution_linepay.reg_key already exists, skipping column.';
  }

  $idxCheck = CRM_Core_DAO::executeQuery("SHOW INDEX FROM `civicrm_contribution_linepay` WHERE Key_name = 'index_reg_key'");
  if ($idxCheck->N === 0) {
    CRM_Core_DAO::executeQuery("ALTER TABLE `civicrm_contribution_linepay` ADD INDEX `index_reg_key` (`reg_key`)");
    $messages[] = 'Added index_reg_key on civicrm_contribution_linepay.reg_key.';
  }
  else {
    $messages[] = 'index_reg_key already exists, skipping index.';
  }

  return implode(' ', $messages);
}
