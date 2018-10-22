<?php
function _neticrm_update_update_7256() {
  $q = CRM_Core_DAO::executeQuery("SHOW columns from `civicrm_price_field` where field = 'max_value'");
  if($q->N == 0){
    $sql = "ALTER TABLE `civicrm_price_field` ADD `max_value` int(10) unsigned DEFAULT NULL COMMENT 'Max number of participants per field' AFTER `is_enter_qty`";
    CRM_Core_DAO::executeQuery($sql);
  }else{
    print("`max_value` is exists on `civicrm_price_field`.\n");
  }
}
