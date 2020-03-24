<?php
/**
 * @file
 * This file should be civicrm compliant.
 */
function _neticrm_update_update_7276() {
  $bouncese = array(
    'DNS' => array(
      'name service error',
    ),
    'Host' => array(
      'service unavailable',
      'domain is not allowed',
      '550.+domain.+facebook.+no longer available',
    ),
    'Inactive' => array(
      '550 user.+disabled',
      'tried to reach is disabled',
      'mailbox is disabled',
    ),
    'Invalid' => array(
      '550.+address rejected',
      '550 user.+unknown',
      'no valid recipient',
      'tried to reach does not exist',
    ),
    'Quota' => array(
      'quota.+not enough',
    ),
    'Relay' => array(
      '550 relaying.+denied',
      '550 relaying.+not allowed',
    ),
  );
  foreach($bouncese as $type => $patterns) {
    $typeId = CRM_Core_DAO::singleValueQuery("SELECT id FROM civicrm_mailing_bounce_type WHERE name LIKE %1", array(1 => array($type, 'String')));
    if ($typeId) {
      foreach($patterns as $pattern) {
        $exists = CRM_Core_DAO::singleValueQuery("SELECT id FROM civicrm_mailing_bounce_pattern WHERE pattern LIKE %1", array(1 => array($pattern, 'String')));
        if (!$exists) {
          CRM_Core_DAO::executeQuery("INSERT INTO civicrm_mailing_bounce_pattern (bounce_type_id, pattern) VALUES (%1, %2)", array(
            1 => array($typeId, 'Positive'),
            2 => array($pattern, 'String'),
          ));
        }
      }
    }
  }
}