<?php
function _neticrm_update_update_7266(){
  CRM_Core_DAO::executeQuery("ALTER TABLE `civicrm_track` ADD INDEX `index_referrer_url` (`referrer_url`)");
  CRM_Core_DAO::executeQuery("ALTER TABLE `civicrm_track` ADD INDEX `index_landing` (`landing`)");
  CRM_Core_DAO::executeQuery("ALTER TABLE `civicrm_track` ADD INDEX `index_utm_campaign` (`utm_campaign`)");
  CRM_Core_DAO::executeQuery("ALTER TABLE `civicrm_track` ADD INDEX `index_utm_term` (`utm_term`)");
  CRM_Core_DAO::executeQuery("ALTER TABLE `civicrm_track` ADD INDEX `index_utm_content` (`utm_content`)");
}
