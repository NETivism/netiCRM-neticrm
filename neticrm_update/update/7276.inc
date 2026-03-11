<?php
/**
 * @file
 * This file should be civicrm compliant.
 */
function _neticrm_update_update_7276() {
  $bouncese = [
    'DNS' => [
      'name service error',
    ],
    'Host' => [
      'service unavailable',
      'domain is not allowed',
      '550.+domain.+facebook.+no longer available',
    ],
    'Inactive' => [
      '550 user.+disabled',
      'tried to reach is disabled',
      'mailbox is disabled',
    ],
    'Invalid' => [
      '550.+address rejected',
      '550 user.+unknown',
      'no valid recipient',
      'tried to reach does not exist',
    ],
    'Quota' => [
      'quota.+not enough',
    ],
    'Relay' => [
      '550 relaying.+denied',
      '550 relaying.+not allowed',
    ],
  ];
  foreach ($bouncese as $type => $patterns) {
    $typeId = CRM_Core_DAO::singleValueQuery("SELECT id FROM civicrm_mailing_bounce_type WHERE name LIKE %1", [1 => [$type, 'String']]);
    if ($typeId) {
      foreach ($patterns as $pattern) {
        $exists = CRM_Core_DAO::singleValueQuery("SELECT id FROM civicrm_mailing_bounce_pattern WHERE pattern LIKE %1", [1 => [$pattern, 'String']]);
        if (!$exists) {
          CRM_Core_DAO::executeQuery("INSERT INTO civicrm_mailing_bounce_pattern (bounce_type_id, pattern) VALUES (%1, %2)", [
            1 => [$typeId, 'Positive'],
            2 => [$pattern, 'String'],
          ]);
        }
      }
    }
  }
}
