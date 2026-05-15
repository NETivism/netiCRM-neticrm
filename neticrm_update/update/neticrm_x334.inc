<?php
function _neticrm_update_update_x334() {
  $exists = CRM_Core_DAO::singleValueQuery("
    SELECT COUNT(*)
    FROM information_schema.COLUMNS
    WHERE TABLE_SCHEMA = DATABASE()
      AND TABLE_NAME = 'civicrm_contribution_page'
      AND COLUMN_NAME = 'redirect_page_id'
  ");
  if ($exists) {
    return 'Column redirect_page_id already exists, skipping.';
  }
  CRM_Core_DAO::executeQuery("
    ALTER TABLE civicrm_contribution_page
    ADD COLUMN redirect_page_id INT UNSIGNED DEFAULT NULL
    COMMENT 'Target contribution page id to redirect when this page is disabled. NULL means show unavailable notice.'
  ");
  return 'Column redirect_page_id added to civicrm_contribution_page.';
}
