<?php
/**
 * @file
 * This file should be civicrm compliant.
 */
function _neticrm_update_update_6138() {
  civicrm_initialize();
  $check_query = "SELECT column_name FROM INFORMATION_SCHEMA.COLUMNS WHERE table_name = 'civicrm_msg_template' AND column_name = 'is_sms'";
  $exists = CRM_Core_DAO::singleValueQuery($check_query);
  if(!$exists){
    $alter_query = "ALTER TABLE `civicrm_msg_template` ADD `is_sms` boolean NULL COMMENT 'Is this message template used for sms?' AFTER `is_reserved`";
    CRM_Core_DAO::executeQuery($alter_query);
  }

  $check_query = "SELECT column_name FROM INFORMATION_SCHEMA.COLUMNS WHERE table_name = 'civicrm_mailing' AND column_name = 'sms_provider_id'";
  $exists = CRM_Core_DAO::singleValueQuery($check_query);
  if(!$exists){
    $alter_query = "ALTER TABLE `civicrm_mailing` ADD `sms_provider_id` int NULL AFTER `dedupe_email`";
    CRM_Core_DAO::executeQuery($alter_query);
  }

  $check_query = "SELECT table_name FROM INFORMATION_SCHEMA.TABLES WHERE table_name = 'civicrm_sms_provider'";
  $exists = CRM_Core_DAO::singleValueQuery($check_query);
  if(!$exists){
    $create_query = "CREATE TABLE civicrm_sms_provider (
         id int unsigned NOT NULL AUTO_INCREMENT  COMMENT 'SMS Provider ID',
         name varchar(64)    COMMENT 'Provider internal name points to option_value of option_group sms_provider_name',
         title varchar(64)    COMMENT 'Provider name visible to user',
         username varchar(255)    ,
         password varchar(255)    ,
         api_type int unsigned NOT NULL   COMMENT 'points to value in civicrm_option_value for group sms_api_type',
         api_url varchar(128)    ,
         api_params text    COMMENT 'the api params in xml, http or smtp format',
         is_default tinyint   DEFAULT 0 ,
         is_active tinyint   DEFAULT 0 ,
         domain_id int unsigned    COMMENT 'Which Domain is this sms provider for' ,
         PRIMARY KEY ( id ),
         CONSTRAINT FK_civicrm_sms_provider_domain_id FOREIGN KEY (domain_id) REFERENCES civicrm_domain(id) ON DELETE SET NULL  
    )  ENGINE=InnoDB DEFAULT CHARACTER SET utf8 COLLATE utf8_unicode_ci ROW_FORMAT=DYNAMIC  ;";
    CRM_Core_DAO::executeQuery($create_query);
  }

  $check_query = "SELECT name FROM civicrm_option_group WHERE name = 'sms_api_type'";
  $exists = CRM_Core_DAO::singleValueQuery($check_query);
  if(!$exists){
    $query = "INSERT INTO 
      `civicrm_option_group` (`name`, `description`, `is_reserved`, `is_active`) VALUES 
      ('sms_api_type'                  , 'Api Type'                           , 0, 1);";
    CRM_Core_DAO::executeQuery($query);

    $query = "SELECT max(id) sms_api_type_group_id from civicrm_option_group where name = 'sms_api_type';";
    $sms_api_type_group_id = CRM_Core_DAO::singleValueQuery($query);

    $insert_query = "INSERT INTO 
       `civicrm_option_value` (`option_group_id`, `label`, `value`, `name`, `grouping`, `filter`, `is_default`, `weight`, `description`, `is_optgroup`, `is_reserved`, `is_active`, `component_id`, `visibility_id`) 
      VALUES
       ($sms_api_type_group_id, 'http', 1, 'http', NULL, 0, 1, 1, NULL, 0, 1, 1, NULL, NULL),
       ($sms_api_type_group_id, 'xml',  2, 'xml',  NULL, 0, 0, 2, NULL, 0, 1, 1, NULL, NULL),
       ($sms_api_type_group_id, 'smtp', 3, 'smtp', NULL, 0, 0, 3, NULL, 0, 1, 1, NULL, NULL);";
     CRM_Core_DAO::executeQuery($insert_query);
  }


  $check_query = "SELECT name FROM civicrm_option_group WHERE name = 'sms_provider_name'";
  $exists = CRM_Core_DAO::singleValueQuery($check_query);
  if(!$exists){
    $query = "INSERT INTO 
      `civicrm_option_group` (`name`, `description`, `is_reserved`, `is_active`) VALUES
      ('sms_provider_name'             , 'Sms Provider Internal Name'         , 0, 1);";
    CRM_Core_DAO::executeQuery($query);
    $query = "SELECT max(id) sms_provider_name_group_id from civicrm_option_group where name = 'sms_provider_name';";
    $sms_provider_name_group_id = CRM_Core_DAO::singleValueQuery($query);

    $insert_query = "INSERT INTO 
       `civicrm_option_value` (`option_group_id`, `label`, `value`, `name`, `grouping`, `filter`, `is_default`, `weight`, `description`, `is_optgroup`, `is_reserved`, `is_active`, `component_id`, `visibility_id`) 
      VALUES
       ($sms_provider_name_group_id , 'Mitake', 'CRM_SMS_Provider_Mitake', 'http', NULL, 0, NULL, 1, NULL, 0, 1, 1, NULL, NULL);";
    CRM_Core_DAO::executeQuery($insert_query);
  }


  return 'Add SMS function to database sucessed.';
}