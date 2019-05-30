<?php
function _neticrm_update_update_6165(){
  $dao = CRM_Core_DAO::executeQuery("SHOW columns from `civicrm_contact` where field = 'created_date'");
  if ($dao->N == 0) {
    $sql = "ALTER TABLE `civicrm_contact` ADD `created_date` DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP AFTER `is_deleted`, ADD INDEX `index_created_date` (`created_date`);";
    CRM_Core_DAO::executeQuery($sql);

    // add default values
    $sql = "SELECT entity_id as contact_id, MIN(modified_date) as created_date FROM `civicrm_log` WHERE `entity_table` LIKE 'civicrm_contact' GROUP BY entity_id ORDER BY entity_id ASC";
    $dao = CRM_Core_DAO::executeQuery($sql);
    while($dao->fetch()) {
      $params = array(
        1 => array($dao->created_date, 'String'),
        2 => array($dao->contact_id, 'Integer'),
      );
      CRM_Core_DAO::executeQuery("UPDATE civicrm_contact SET created_date = %1 WHERE id = %2", $params);
    }
  }

  $dao = CRM_Core_DAO::executeQuery("SHOW columns from `civicrm_contact` where field = 'modified_date'");
  if ($dao->N == 0) {
    $sql = "ALTER TABLE `civicrm_contact` ADD `modified_date` DATETIME on update CURRENT_TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP AFTER `created_date`, ADD INDEX `index_modified_date` (`modified_date`);";
    CRM_Core_DAO::executeQuery($sql);

    $sql = "SELECT entity_id as contact_id, MAX(modified_date) as modified_date FROM `civicrm_log` WHERE `entity_table` LIKE 'civicrm_contact' GROUP BY entity_id ORDER BY entity_id ASC";
    $dao = CRM_Core_DAO::executeQuery($sql);
    while($dao->fetch()) {
      $params = array(
        1 => array($dao->modified_date, 'String'),
        2 => array($dao->contact_id, 'Integer'),
      );
      CRM_Core_DAO::executeQuery("UPDATE civicrm_contact SET modified_date = %1 WHERE id = %2", $params);
    }
  }
}
