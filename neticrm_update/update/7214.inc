<?php
/**
 * @file
 * Update for 7214
 *
 * This file should be civicrm compliant.
 */
function _neticrm_update_update_7214(){
  $check_query = "SELECT column_name FROM INFORMATION_SCHEMA.COLUMNS WHERE table_name = 'civicrm_uf_field' AND column_name = 'website_type_id'";
  $exists = CRM_Core_DAO::singleValueQuery($check_query);

  if (!$exists) {
    CRM_Core_DAO::executeQuery("ALTER TABLE civicrm_uf_field ADD COLUMN `website_type_id` int(10) unsigned DEFAULT NULL COMMENT 'Website Type Id, if required' AFTER phone_type_id, ADD INDEX `IX_website_type_id` (`website_type_id`)");

    $types = CRM_Core_PseudoConstant::websiteType();
    $website_type_id = key($types);
    $params = array(
      1 => array($website_type_id, 'Positive'),
    );
    CRM_Core_DAO::executeQuery("UPDATE civicrm_uf_field SET website_type_id = %1 WHERE field_name = 'url'", $params);
  }

  return "Success add website type id into uf field";
}
