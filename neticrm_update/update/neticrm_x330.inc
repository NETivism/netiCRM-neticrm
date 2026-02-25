<?php

function _neticrm_update_update_x330() {
  civicrm_initialize();

  // Check if civicrm_product.stock_status default value is 1
  $query = "SELECT COLUMN_DEFAULT
            FROM INFORMATION_SCHEMA.COLUMNS
            WHERE TABLE_SCHEMA = DATABASE()
            AND TABLE_NAME = 'civicrm_product'
            AND COLUMN_NAME = 'stock_status'";

  $dao = CRM_Core_DAO::executeQuery($query);

  if ($dao->fetch()) {
    // If default value is 1, change it to 0
    if ($dao->COLUMN_DEFAULT === '1' || $dao->COLUMN_DEFAULT == 1) {
      $alterQuery = "ALTER TABLE civicrm_product
                     MODIFY COLUMN stock_status tinyint(4) DEFAULT 0 COMMENT 'Stock management status for this product.'";
      CRM_Core_DAO::executeQuery($alterQuery);
      echo "civicrm_product.stock_status default value changed from 1 to 0.";
    }
    else {
      echo "civicrm_product.stock_status default value is already {$dao->COLUMN_DEFAULT}, no changes needed.";
    }
  }
  else {
    echo "civicrm_product.stock_status column not found.";
  }
}