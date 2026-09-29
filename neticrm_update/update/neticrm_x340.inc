<?php

function _neticrm_update_update_x340() {
  civicrm_initialize();

  // Rows written between the x339 release and the multi turn backend have no
  // conversation_id: the old backend never touched the column. x339 already
  // backfilled everything that existed at its own run time, so this second
  // pass only matters when the two releases go out separately.
  $pending = CRM_Core_DAO::singleValueQuery("SELECT COUNT(*) FROM `civicrm_aicompletion` WHERE `conversation_id` IS NULL");
  if ($pending > 0) {
    CRM_Core_DAO::executeQuery("UPDATE `civicrm_aicompletion` SET `conversation_id` = `id` WHERE `conversation_id` IS NULL");
    return "Backfilled conversation_id on {$pending} row(s).";
  }

  return 'No row needs conversation_id backfill.';
}
