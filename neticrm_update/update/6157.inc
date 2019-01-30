<?php
function _neticrm_update_update_6157() {
  $q = CRM_Core_DAO::executeQuery("SHOW columns from `civicrm_contribution_page` where field = 'background_URL'");
  if($q->N == 0){
    $sql = "ALTER TABLE `civicrm_contribution_page` ADD `background_URL` varchar(255) DEFAULT NULL COMMENT 'Background image url on contribution page of special style.' AFTER `currency`";
    CRM_Core_DAO::executeQuery($sql);
  }else{
    print("`background_URL` is exists on `civicrm_contribution_page`.\n");
  }

  $q = CRM_Core_DAO::executeQuery("SHOW columns from `civicrm_contribution_page` where field = 'mobile_background_URL'");
  if($q->N == 0){
    $sql = "ALTER TABLE `civicrm_contribution_page` ADD `mobile_background_URL` varchar(255) DEFAULT NULL COMMENT 'Background image url on contribution page of special style in mobile.' AFTER `background_URL`";
    CRM_Core_DAO::executeQuery($sql);
  }else{
    print("`mobile_background_URL` is exists on `civicrm_contribution_page`.\n");
  }
}
