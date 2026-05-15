<?php
function _neticrm_update_update_x336() {
  $exists = CRM_Core_DAO::singleValueQuery("
    SELECT COUNT(*)
    FROM information_schema.COLUMNS
    WHERE TABLE_SCHEMA = DATABASE()
      AND TABLE_NAME = 'civicrm_price_set'
      AND COLUMN_NAME = 'show_remaining'
  ");
  if ($exists) {
    return 'Column show_remaining already exists in civicrm_price_set, skipping.';
  }
  CRM_Core_DAO::executeQuery("
    ALTER TABLE civicrm_price_set
    ADD COLUMN show_remaining TINYINT DEFAULT 0
    COMMENT 'Show remaining capacity on frontend price field options'
  ");
  return 'Column show_remaining added to civicrm_price_set.';
}
