<?php

function _neticrm_update_update_6180() {
  $dao = CRM_Core_DAO::executeQuery("SHOW COLUMNS FROM civicrm_contribution_page WHERE FIELD like 'is_send_sms'");
  $dao->fetch();
  if (empty($dao->Field)) {
    $sql = "ALTER TABLE `civicrm_contribution_page` ADD `is_send_sms` tinyint DEFAULT 0 COMMENT 'if true, it will automatically send mobile SMS message to successful donor.' AFTER `mobile_background_URL`";
    CRM_Core_DAO::executeQuery($sql);
  }

  $dao = CRM_Core_DAO::executeQuery("SHOW COLUMNS FROM civicrm_contribution_page WHERE FIELD like 'sms_text'");
  $dao->fetch();
  if (empty($dao->Field)) {
    $sql = "ALTER TABLE `civicrm_contribution_page` ADD `sms_text` text COMMENT 'SMS message content.' AFTER `is_send_sms`";
    CRM_Core_DAO::executeQuery($sql);
  }
}