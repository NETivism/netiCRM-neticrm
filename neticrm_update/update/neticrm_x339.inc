<?php

function _neticrm_update_update_x339() {
  civicrm_initialize();

  $messages = [];

  // conversation_id column. No FK on purpose: it points to the first row of the
  // same thread, so deleting that row would orphan the whole conversation.
  $colCheck = CRM_Core_DAO::executeQuery("SHOW COLUMNS FROM `civicrm_aicompletion` LIKE 'conversation_id'");
  if ($colCheck->N === 0) {
    $sql = <<<EOT
      ALTER TABLE `civicrm_aicompletion`
      ADD COLUMN `conversation_id` int unsigned DEFAULT NULL COMMENT 'Groups rows of the same conversation. Points to the id of the first row in the thread.' AFTER `contact_id`
    EOT;
    CRM_Core_DAO::executeQuery($sql);
    $messages[] = 'Added conversation_id to civicrm_aicompletion.';
  }
  else {
    $messages[] = 'civicrm_aicompletion.conversation_id already exists, skipping column.';
  }

  $idxCheck = CRM_Core_DAO::executeQuery("SHOW INDEX FROM `civicrm_aicompletion` WHERE Key_name = 'index_conversation_id'");
  if ($idxCheck->N === 0) {
    CRM_Core_DAO::executeQuery("ALTER TABLE `civicrm_aicompletion` ADD INDEX `index_conversation_id` (`conversation_id`)");
    $messages[] = 'Added index_conversation_id on civicrm_aicompletion.conversation_id.';
  }
  else {
    $messages[] = 'index_conversation_id already exists, skipping index.';
  }

  // Backfill: every existing row becomes a single turn conversation of its own.
  // Kept outside the column check above so rows written before the multi turn
  // backend goes live also get filled when this update runs again.
  $pending = CRM_Core_DAO::singleValueQuery("SELECT COUNT(*) FROM `civicrm_aicompletion` WHERE `conversation_id` IS NULL");
  if ($pending > 0) {
    CRM_Core_DAO::executeQuery("UPDATE `civicrm_aicompletion` SET `conversation_id` = `id` WHERE `conversation_id` IS NULL");
    $messages[] = "Backfilled conversation_id on {$pending} row(s).";
  }
  else {
    $messages[] = 'No row needs conversation_id backfill.';
  }

  return implode(' ', $messages);
}
