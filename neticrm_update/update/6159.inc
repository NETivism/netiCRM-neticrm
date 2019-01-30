<?php
function _neticrm_update_update_6159() {
  $q = CRM_Core_DAO::executeQuery("SHOW columns from `civicrm_coupon_track` where field = 'discount_amount'");
  if($q->N == 0){
    $sql = "ALTER TABLE `civicrm_coupon_track` ADD `discount_amount` int(10) unsigned DEFAULT NULL COMMENT 'Total discount amount on contribution' AFTER `used_date`";
    CRM_Core_DAO::executeQuery($sql);
  }else{
    print("`discount_amount` is exists on `civicrm_coupon_track`.\n");
  }
}
