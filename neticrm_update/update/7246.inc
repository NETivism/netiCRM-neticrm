<?php

function _neticrm_update_update_7246(){
  // Change bounce_reason to text
  $sql = "ALTER TABLE `civicrm_mailing_event_bounce` CHANGE `bounce_reason` `bounce_reason` TEXT NULL DEFAULT NULL COMMENT 'The reason the email bounced.'";
  CRM_Core_DAO::executeQuery($sql);

  // host or domain name not found
  $exists = CRM_Core_DAO::singleValueQuery("SELECT id FROM civicrm_mailing_bounce_pattern WHERE pattern like 'host or domain name not found'");
  if (!$exists) {
    $sql = "INTO civicrm_mailing_bounce_pattern (`bounce_type_id`, `pattern`) VALUES (4, 'host or domain name not found')";
    CRM_Core_DAO::executeQuery($sql);
  }

  // update syntax error threshold and description
  $syntax = ts('Error in SMTP transaction');
  $sql = "UPDATE civicrm_mailing_bounce_type SET description = '$syntax', hold_threshold = 100 WHERE name = 'Syntax'";
  CRM_Core_DAO::executeQuery($sql);

  return 'Successful add goal_recurring into contirbution page';
}
