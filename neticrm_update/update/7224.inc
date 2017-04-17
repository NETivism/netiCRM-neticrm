<?php
/**
 * @file
 * This file should be civicrm compliant.
 */
function _neticrm_update_update_7224() {
  $sql = "UPDATE civicrm_contribution c SET c.cancel_reason = NULL WHERE c.cancel_reason LIKE '0'";
  CRM_Core_DAO::executeQuery($sql);

  // use created date as cancel date when no receive date
  $sql = "UPDATE civicrm_contribution c LEFT JOIN civicrm_note n ON c.id = n.entity_id AND n.entity_table = 'civicrm_contribution' SET c.cancel_date = c.created_date, c.cancel_reason = n.note, receive_date = NULL WHERE c.contribution_status_id = 4 AND c.cancel_date IS NULL AND (c.receive_date IS NULL OR c.receive_date = '0000-00-00 00:00:00') AND c.payment_processor_id IS NOT NULL";
  CRM_Core_DAO::executeQuery($sql);

  // move receive date to cancel date when recurring contribution is failed state
  $sql = "UPDATE civicrm_contribution c LEFT JOIN civicrm_note n ON c.id = n.entity_id AND n.entity_table = 'civicrm_contribution' SET c.cancel_date = c.receive_date, c.cancel_reason = n.note, c.receive_date = NULL WHERE c.contribution_status_id = 4 AND c.receive_date IS NOT NULL AND c.cancel_date IS NULL AND c.payment_processor_id IS NOT NULL";
  CRM_Core_DAO::executeQuery($sql);

  return 'Successful update recurring contribution cancel date.';
}
