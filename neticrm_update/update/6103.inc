<?php
/**
 * @file
 * Update for 6103
 *
 * This file should be civicrm compliant.
 */
function _neticrm_update_update_6103(){
  $query = "SELECT id FROM civicrm_dedupe_rule_group WHERE contact_type = 'Individual' AND  level = 'Strict' AND is_default = 1";
  $dedupe_group_id = CRM_Core_DAO::singleValueQuery($query);
  $query = "SELECT * FROM civicrm_dedupe_rule WHERE dedupe_rule_group_id = 4";
  $dao = CRM_Core_DAO::executeQuery($query);
  $dao->fetch();
  if($dao->N === 1){
    CRM_Core_DAO::executeQuery("UPDATE civicrm_dedupe_rule_group SET threshold = 20 WHERE id = $dedupe_group_id");
    CRM_Core_DAO::executeQuery("DELETE FROM `civicrm_dedupe_rule` WHERE dedupe_rule_group_id = $dedupe_group_id");
    CRM_Core_DAO::executeQuery("INSERT INTO civicrm_dedupe_rule (dedupe_rule_group_id, rule_table, rule_field, rule_weight) VALUES ($dedupe_group_id, 'civicrm_contact', 'first_name', 5), ($dedupe_group_id, 'civicrm_contact', 'last_name',  7), ($dedupe_group_id, 'civicrm_email', 'email', 10)");
    return "Successful update dedupe rule.";
  }
  return "Dedupe rule is fine. We don't update it.";
}
