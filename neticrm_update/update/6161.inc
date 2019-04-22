<?php
function _neticrm_update_update_6161() {
  $q = CRM_Core_DAO::executeQuery("SHOW columns from `civicrm_contribution_recur` where field = 'last_execute_date'");
  if($q->N == 0){
    $sql = "ALTER TABLE `civicrm_contribution_recur` ADD `last_execute_date` datetime DEFAULT NULL COMMENT 'Last expexted execute transaction date.' AFTER `auto_renew`";
    CRM_Core_DAO::executeQuery($sql);
  }else{
    print("`last_execute_date` is exists on `civicrm_contribution_recur`.\n");
  }
}

