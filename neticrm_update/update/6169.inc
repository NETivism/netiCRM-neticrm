<?php
function _neticrm_update_update_6169(){
  // wrong traffic source on these date #25894
  $sql = "SHOW COLUMNS FROM civicrm_event WHERE field LIKE 'is_qrcode'";
  $dao = CRM_Core_DAO::executeQuery($sql);
  if (empty($dao->N)) {
    $sql = "ALTER TABLE `civicrm_event` ADD `is_qrcode` tinyint UNSIGNED DEFAULT 0 COMMENT 'If true, include qrcode image on event registered email notification.' AFTER is_online_registration ";
    $dao = CRM_Core_DAO::executeQuery($sql);
  }
}
