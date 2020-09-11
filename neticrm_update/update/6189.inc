<?php

function _neticrm_update_update_6189() {
  $dao = CRM_Core_DAO::executeQuery("SHOW COLUMNS FROM civicrm_membership_block WHERE FIELD like 'is_renewal_only'");
  $dao->fetch();
  if (empty($dao->Field)) {
    $sql = "ALTER TABLE `civicrm_membership_block` ADD `is_renewal_only` tinyint DEFAULT 0 COMMENT 'Is this membership_block only used for renewal' AFTER `is_active`";
    CRM_Core_DAO::executeQuery($sql);
  }
}