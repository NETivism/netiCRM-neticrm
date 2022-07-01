<?php

/**
 * Example function to doing database update
 *
 * @return mixed Success: NULL, TRUE, String. Failed: FALSE or string contain "[error]"
 */
function _neticrm_update_update_x306() {
  $dao = CRM_Core_DAO::executeQuery("SELECT data_type FROM information_schema.columns WHERE `table_name` = 'civicrm_mailing' AND column_name = 'is_hidden'");
  $dao->fetch();
  if ($dao->data_type !== 'tinyint' && empty($dao->data_type)) {
    $sql = "ALTER TABLE `civicrm_mailing` ADD `is_hidden` TINYINT DEFAULT '0' COMMENT 'Is this mailing hidden?' AFTER `sms_provider_id`";
    CRM_Core_DAO::executeQuery($sql);
  }

  // add activity types
  _neticrm_update_add_activity_type_x306('Contribution Notification Email', 'CiviContribute');
  _neticrm_update_add_activity_type_x306('Event Notification Email', 'CiviEvent');
  _neticrm_update_add_activity_type_x306('Membership Notification Email', 'CiviMember');
  _neticrm_update_add_activity_type_x306('PCP Notification Email', '');
  _neticrm_update_add_activity_type_x306('Mailing Notification Email', '');

  // add default transactional mailing
  $mailing = new CRM_Mailing_DAO_Mailing();
  $mailing->is_hidden = 1;
  $mailing->name = 'Transactional Email';
  if (!$mailing->find(TRUE)) {
    $params = array(
      'header_id' => 'null',
      'footer_id' => 'null',
      'reply_id' => 'null',
      'unsubscribe_id' => 'null',
      'resubscribe_id' => 'null',
      'optout_id' => 'null',
      'name' => 'Transactional Email',
      'from_name' => '',
      'from_email' => '',
      'replyto_email' => '',
      'subject' => '',
      'body_preview' => 'null',
      'body_text' => 'null',
      'body_html' => 'null',
      //'body_json' => '',
      'url_tracking' => FALSE,
      'forward_replies' => FALSE,
      'auto_responder' => FALSE,
      'open_tracking' => TRUE,
      'msg_template_id' => 'null',
      'override_verp' => FALSE,
      'created_id' => 'null',
      'created_date' => date('YmdHis'),
      'scheduled_id' => 'null',
      'scheduled_date' => 'null',
      'is_archived' => 0,
      'visibility' => 'User and User Admin Only',
      'dedupe_email' => 0,
      'sms_provider_id' => 'null',
      'is_hidden' => 1,
    );
    $mailing = CRM_Mailing_BAO_Mailing::add($params);
  }

  $job = new CRM_Mailing_DAO_Job();
  $job->mailing_id = $mailing->id;
  $job->is_test = 0;
  if (!$job->find(TRUE)) {
    $params = array(
      'mailing_id' => $mailing->id,
      'status' => 'Running',
      'scheduled_date' => 'null',
      'is_test' => 0,
    );
    $job = CRM_Mailing_BAO_Job::create($params);
  }

  // add event transactional table
  $exists = CRM_Core_DAO::singleValueQuery("SELECT table_name FROM INFORMATION_SCHEMA.TABLES WHERE table_name = 'civicrm_mailing_event_transactional'");
  if(!$exists){
    $sql = "CREATE TABLE civicrm_mailing_event_transactional (
      id int unsigned NOT NULL AUTO_INCREMENT  ,
      event_queue_id int unsigned NOT NULL   COMMENT 'FK to EventQueue',
      activity_id int unsigned NOT NULL   COMMENT 'Foreign key to the activity for this mailing deliver record.',
      PRIMARY KEY ( id ),
      CONSTRAINT FK_civicrm_mailing_event_transactional_event_queue_id FOREIGN KEY (event_queue_id) REFERENCES civicrm_mailing_event_queue(id) ON DELETE CASCADE,
      CONSTRAINT FK_civicrm_mailing_event_transactional_activity_id FOREIGN KEY (activity_id) REFERENCES civicrm_activity(id) ON DELETE CASCADE
  )  ENGINE=InnoDB DEFAULT CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci ROW_FORMAT=DYNAMIC";
    CRM_Core_DAO::executeQuery($sql);
  }
}

function _neticrm_update_add_activity_type_x306($type, $component) {
  $optionGroupId = CRM_Core_DAO::singleValueQuery("SELECT id FROM civicrm_option_group WHERE name = 'activity_type'");

  $exists = CRM_Core_DAO::singleValueQuery("SELECT name FROM civicrm_option_value WHERE name = %1 AND option_group_id = %2", array(
    1 => array($type, 'String'),
    2 => array($optionGroupId, 'Positive')
  ));
  if (!$exists) {
    $maxValue = CRM_Core_DAO::singleValueQuery("SELECT MAX(CAST(value AS UNSIGNED)) FROM civicrm_option_value WHERE option_group_id = %1", array(1 => array($optionGroupId, 'Positive')));
    $params = array(
      1 => array($optionGroupId, 'Positive'),
      2 => array($maxValue + 1, 'Positive'),
      3 => array(ts($type), 'String'),
      4 => array($type, 'String'),
    );
    if (!empty($component)) {
      $componentId = CRM_Core_DAO::singleValueQuery("SELECT MAX(id) FROM civicrm_component WHERE name = %1", array(1 => array($component, 'String')));
      $params[5] = array($componentId, 'Positive');
      $sql = "INSERT INTO `civicrm_option_value` (`option_group_id`, `label`, `value`, `name`, `grouping`, `filter`, `is_default`, `weight`, `description`, `is_optgroup`, `is_reserved`, `is_active`, `component_id`, `visibility_id`) VALUES (%1, %3, %2, %4, NULL, 1, 0, %2, '', 0, 1, 1, %5, NULL);";
    }
    else {
      $sql = "INSERT INTO `civicrm_option_value` (`option_group_id`, `label`, `value`, `name`, `grouping`, `filter`, `is_default`, `weight`, `description`, `is_optgroup`, `is_reserved`, `is_active`, `component_id`, `visibility_id`) VALUES (%1, %3, %2, %4, NULL, 1, 0, %2, '', 0, 1, 1, NULL, NULL);";
    }
    CRM_Core_DAO::executeQuery($sql, $params);
  }
}
