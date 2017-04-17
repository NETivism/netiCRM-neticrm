<?php
/**
 * @file
 * This file should be civicrm compliant.
 */
function _neticrm_update_update_6124() {
  // use created date as cancel date when no receive date
  $sql = "UPDATE civicrm_contribution c INNER JOIN civicrm_note n ON c.id = n.entity_id AND n.entity_table = 'civicrm_contribution' SET c.cancel_date = c.created_date, c.cancel_reason = n.note WHERE c.contribution_status_id = 4 AND c.cancel_date IS NULL AND c.receive_date IS NULL AND c.contribution_recur_id IS NOT NULL";
  CRM_Core_DAO::executeQuery($sql);

  // move receive date to cancel date when recurring contribution is failed state
  $sql = "UPDATE civicrm_contribution c INNER JOIN civicrm_note n ON c.id = n.entity_id AND n.entity_table = 'civicrm_contribution' SET c.cancel_date = c.receive_date, c.cancel_reason = n.note, c.receive_date = NULL WHERE c.contribution_status_id = 4 AND c.receive_date IS NOT NULL AND c.cancel_date IS NULL AND c.contribution_recur_id IS NOT NULL";
  CRM_Core_DAO::executeQuery($sql);


  return 'Successful update recurring contribution cancel date.';
}
