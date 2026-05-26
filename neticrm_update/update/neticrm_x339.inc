<?php

function _neticrm_update_update_x339() {
  civicrm_initialize();

  // Add editor_allow_all_content column on existing sites. New sites get it via
  // the schema XML (CREATE TABLE); old sites need this ALTER to stay in sync.
  $result = CRM_Core_DAO::executeQuery("SHOW COLUMNS FROM `civicrm_preferences` LIKE 'editor_allow_all_content'");
  if ($result->N === 0) {
    $sql = <<<EOT
      ALTER TABLE `civicrm_preferences`
      ADD COLUMN `editor_allow_all_content` tinyint DEFAULT 0 COMMENT 'If set, WYSIWYG editors allow all HTML content including script tags (XSS risk).'
    EOT;
    CRM_Core_DAO::executeQuery($sql);
    echo "Added editor_allow_all_content to civicrm_preferences.\n";
  }
  else {
    echo "civicrm_preferences editor_allow_all_content already exists, skipping.\n";
  }
}
