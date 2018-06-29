<?php

function _neticrm_update_update_6146(){
  // Change bounce_reason to text
  $sql = "ALTER TABLE `civicrm_mailing_event_bounce` CHANGE `bounce_reason` `bounce_reason` TEXT NULL DEFAULT NULL COMMENT 'The reason the email bounced.'";
  CRM_Core_DAO::executeQuery($sql);

  // host or domain name not found
  $sql = "INSERT INTO civicrm_mailing_bounce_pattern (`bounce_type_id`, `pattern`) VALUES (4, 'host or domain name not found')";
  CRM_Core_DAO::executeQuery($sql);

  return 'Successful add goal_recurring into contirbution page';
}
