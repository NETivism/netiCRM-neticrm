<?php

function _neticrm_update_update_6191() {
  $check_query = "SELECT column_name FROM INFORMATION_SCHEMA.COLUMNS WHERE table_name = 'civicrm_contribution_page' AND column_name = 'is_internal'";
  $exists = CRM_Core_DAO::singleValueQuery($check_query);

  if(!$exists){
    $alter_query = "ALTER TABLE `civicrm_contribution_page` ADD `is_internal` TINYINT COMMENT 'Is this page only for internal usage?' AFTER `is_active`;";
    CRM_Core_DAO::executeQuery($alter_query);
    return "Successful add is_internal column";
  }

  return "Column exists, doesn't need update";
}