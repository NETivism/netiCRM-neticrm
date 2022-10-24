<?php

/**
 * Example function to doing database update
 *
 * @return mixed Success: NULL, TRUE, String. Failed: FALSE or string contain "[error]"
 */
function _neticrm_update_update_x307() {
  $dao = CRM_Core_DAO::executeQuery("SELECT table_name FROM information_schema.columns WHERE `table_name` = 'civicrm_instrument' AND column_name = 'id'");
  $dao->fetch();
  if (!empty($dao->table_name)) {
    $dao = CRM_Core_DAO::executeQuery("SELECT data_type FROM information_schema.columns WHERE `table_name` = 'civicrm_instrument' AND column_name = 'id'");
    $dao->fetch();
    if (empty($dao->data_type)) {
      $sql = "ALTER TABLE `civicrm_instrument` DROP PRIMARY KEY";
      CRM_Core_DAO::executeQuery($sql);
      $sql = "ALTER TABLE `civicrm_instrument` ADD UNIQUE (`entity_id`, `entity_table`)";
      CRM_Core_DAO::executeQuery($sql);
      $sql = "ALTER TABLE `civicrm_instrument` ADD `id` INT UNSIGNED NOT NULL AUTO_INCREMENT COMMENT 'Unique table ID' FIRST, ADD PRIMARY KEY (`id`)";
      CRM_Core_DAO::executeQuery($sql);
    }
  }

  $dao = CRM_Core_DAO::executeQuery("SELECT table_name FROM information_schema.columns WHERE `table_name` = 'civicrm_contribution_allpay'");
  $dao->fetch();
  if (!empty($dao->table_name)) {
    $dao = CRM_Core_DAO::executeQuery("SELECT data_type FROM information_schema.columns WHERE `table_name` = 'civicrm_contribution_allpay' AND column_name = 'id'");
    $dao->fetch();
    if (empty($dao->data_type)) {
      $sql = "ALTER TABLE `civicrm_contribution_allpay` DROP PRIMARY KEY";
      CRM_Core_DAO::executeQuery($sql);
      $sql = "ALTER TABLE `civicrm_contribution_allpay` CHANGE `cid` `cid` INT(10) UNSIGNED NULL COMMENT 'Contribution id for civicrm'";
      CRM_Core_DAO::executeQuery($sql);
      $sql = "ALTER TABLE `civicrm_contribution_allpay` ADD UNIQUE (`cid`)";
      CRM_Core_DAO::executeQuery($sql);
      $sql = "ALTER TABLE `civicrm_contribution_allpay` ADD `id` INT UNSIGNED NOT NULL AUTO_INCREMENT COMMENT 'Unique table ID' FIRST, ADD PRIMARY KEY (`id`)";
      CRM_Core_DAO::executeQuery($sql);
    }
  }
}
