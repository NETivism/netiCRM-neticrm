<?php

function _neticrm_update_update_x337() {
  civicrm_initialize();
  $result = CRM_Core_DAO::executeQuery("SHOW COLUMNS FROM `civicrm_price_set` LIKE 'show_remaining'");
  if ($result->N === 0) {
    $sql = <<<EOT
      ALTER TABLE `civicrm_price_set`
      ADD COLUMN `show_remaining` tinyint DEFAULT 0 COMMENT 'Show remaining capacity on frontend price field options'
    EOT;
    CRM_Core_DAO::executeQuery($sql);
    return 'Added show_remaining to civicrm_price_set.';
  }
  else {
    return 'Column show_remaining already exists in civicrm_price_set, skipping.';
  }
}
