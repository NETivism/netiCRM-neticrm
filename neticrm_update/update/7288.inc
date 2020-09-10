<?php
function _neticrm_update_update_7288() {
  // get first contribution processor id to recurring table, clear processor_id which not numeric
  $sql = "SELECT MAX(c.payment_processor_id) as payment_processor_id, r.id, r.processor_id FROM civicrm_contribution c INNER JOIN civicrm_contribution_recur r ON c.contribution_recur_id = r.id WHERE c.payment_processor_id IS NOT NULL GROUP BY c.contribution_recur_id";
  $dao = CRM_Core_DAO::executeQuery($sql);
  while($dao->fetch()) {
    $sql = "UPDATE civicrm_contribution_recur SET processor_id = %1 WHERE id = %2";
    CRM_Core_DAO::executeQuery($sql, array(
      1 => array($dao->payment_processor_id, 'Integer'),
      2 => array($dao->id, 'Integer'),
    ));
  }

  // remove non-numeric processor_id saved on this field
  $sql = "SELECT id, processor_id FROM `civicrm_contribution_recur` WHERE processor_id NOT RLIKE '^[0-9]*$'";
  $dao = CRM_Core_DAO::executeQuery($sql);
  while($dao->fetch()) {
    $sql = "UPDATE civicrm_contribution_recur SET processor_id = NULL WHERE id = %2";
    CRM_Core_DAO::executeQuery($sql, array(
      2 => array($dao->id, 'Integer'),
    ));
  }


  // update field
  $check_query = "SELECT column_name FROM INFORMATION_SCHEMA.COLUMNS WHERE table_name = 'civicrm_contribution_recur' AND column_name = 'external_id'";
  $exists = CRM_Core_DAO::singleValueQuery($check_query);

  if(!$exists){
    $sql = "ALTER TABLE `civicrm_contribution_recur` CHANGE `processor_id` `processor_id` INT UNSIGNED NULL DEFAULT NULL COMMENT 'FK to payment processor'";
    CRM_Core_DAO::executeQuery($sql);
    $sql = "ALTER TABLE `civicrm_contribution_recur` ADD CONSTRAINT `FK_civicrm_contribution_recur_processor_id` FOREIGN KEY (`processor_id`) REFERENCES `civicrm_payment_processor`(`id`) ON DELETE SET NULL ON UPDATE RESTRICT";
    CRM_Core_DAO::executeQuery($sql);
    $sql = "ALTER TABLE `civicrm_contribution_recur` ADD `external_id` VARCHAR(255) NULL DEFAULT NULL COMMENT 'Possibly needed to store a unique identifier for this recurring payment order - if this is available from the processor??' AFTER `processor_id`";
    CRM_Core_DAO::executeQuery($sql);

    return 'Successful update recur table field';
  }
  return 'Recurring external_id column exists.';
}
