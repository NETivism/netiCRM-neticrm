<?php

function _neticrm_update_update_x326() {
  civicrm_initialize();
  
  // Add premiums_combination field to civicrm_premiums table
  $query = "SHOW COLUMNS FROM `civicrm_premiums` LIKE 'premiums_combination'";
  $result = CRM_Core_DAO::executeQuery($query);
  $column_exists = $result->N > 0;

  if (!$column_exists) {
    $query = <<<EOT
      ALTER TABLE `civicrm_premiums`
      ADD COLUMN `premiums_combination` tinyint(4) DEFAULT 0 COMMENT 'Enable premiums combination feature for this page.'
    EOT;
    CRM_Core_DAO::executeQuery($query);
    echo "Added premiums_combination column to civicrm_premiums table.\n";
  }
  else {
    echo "civicrm_premiums premiums_combination column already exists.\n";
  }

  // Create civicrm_premiums_combination table
  $query = "SHOW TABLES LIKE 'civicrm_premiums_combination'";
  $result = CRM_Core_DAO::executeQuery($query);
  $table_exists = $result->N > 0;
  
  if (!$table_exists) {
    $query = <<<EOT
      CREATE TABLE `civicrm_premiums_combination` (
        `id` int unsigned NOT NULL AUTO_INCREMENT,
        `premiums_id` int unsigned NOT NULL COMMENT 'Foreign key to civicrm_premiums table',
        `combination_name` varchar(255) NOT NULL COMMENT 'Name of the premium combination',
        `description` text DEFAULT NULL COMMENT 'Optional description of the product/premium.',
        `sku` varchar(50) DEFAULT NULL COMMENT 'Optional product sku or code.',
        `min_contribution` decimal(20,2) NOT NULL COMMENT 'Minimum non-recurring contribution amount required for this combination',
        `min_contribution_recur` decimal(20,2) DEFAULT NULL COMMENT 'Minimum recurring contribution amount threshold for this combination',
        `currency` varchar(3) DEFAULT NULL COMMENT '3 character string, value from config setting or input via user.',
        `is_active` tinyint(4) DEFAULT 1 COMMENT 'Whether this combination is active',
        `weight` int unsigned DEFAULT 0 COMMENT 'Order for displaying combinations',
        `calculate_mode` varchar(32) DEFAULT NULL COMMENT 'Calculate by accumulate or by min-amount of contribution',
        `installments` int DEFAULT 0 COMMENT 'Num of installments when calculate mode is accumulate.',
        `image` varchar(255) DEFAULT NULL COMMENT 'Full or relative URL to uploaded image - fullsize.',
        `thumbnail` varchar(255) DEFAULT NULL COMMENT 'Full or relative URL to image thumbnail.',
        `created_date` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP COMMENT 'When was this combination created',
        `modified_date` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP COMMENT 'When was this combination last modified',
        PRIMARY KEY (`id`),
        KEY `FK_civicrm_premiums_combination_premiums_id` (`premiums_id`),
        CONSTRAINT `FK_civicrm_premiums_combination_premiums_id` FOREIGN KEY (`premiums_id`) REFERENCES `civicrm_premiums` (`id`) ON DELETE CASCADE
      ) ENGINE=InnoDB DEFAULT CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci ROW_FORMAT=DYNAMIC COMMENT='Table for storing premiums combination configurations'
    EOT;
    CRM_Core_DAO::executeQuery($query);
    echo "Created civicrm_premiums_combination table.\n";
  }
  else {
    echo "civicrm_premiums_combination table already exists.\n";
  }

  // Create civicrm_premiums_combination_products table
  $query = "SHOW TABLES LIKE 'civicrm_premiums_combination_products'";
  $result = CRM_Core_DAO::executeQuery($query);
  $table_exists = $result->N > 0;
  
  if (!$table_exists) {
    $query = <<<EOT
      CREATE TABLE `civicrm_premiums_combination_products` (
        `id` int unsigned NOT NULL AUTO_INCREMENT,
        `combination_id` int unsigned NOT NULL COMMENT 'Foreign key to civicrm_premiums_combination table',
        `product_id` int unsigned NOT NULL COMMENT 'Foreign key to civicrm_product table',
        `quantity` int unsigned DEFAULT 1 COMMENT 'Quantity of this product in the combination',
        PRIMARY KEY (`id`),
        CONSTRAINT `FK_civicrm_premiums_combination_products_combination_id` FOREIGN KEY (`combination_id`) REFERENCES `civicrm_premiums_combination` (`id`) ON DELETE CASCADE,
        CONSTRAINT `FK_civicrm_premiums_combination_products_product_id` FOREIGN KEY (`product_id`) REFERENCES `civicrm_product` (`id`) ON DELETE CASCADE
      ) ENGINE=InnoDB DEFAULT CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci ROW_FORMAT=DYNAMIC
    EOT;
    CRM_Core_DAO::executeQuery($query);
    echo "Created civicrm_premiums_combination_products table.\n";
  }
  else {
    echo "civicrm_premiums_combination_products table already exists.\n";
  }

  // Add stock management fields to civicrm_product table
  $query = "SHOW COLUMNS FROM `civicrm_product` LIKE 'stock_status'";
  $result = CRM_Core_DAO::executeQuery($query);
  $column_exists = $result->N > 0;

  if (!$column_exists) {
    $query = <<<EOT
      ALTER TABLE `civicrm_product`
      ADD COLUMN `stock_status` tinyint(4) DEFAULT 1 COMMENT 'Stock management status for this product.',
      ADD COLUMN `stock_qty` int DEFAULT 0 COMMENT 'Available stock quantity for this product.',
      ADD COLUMN `send_qty` int DEFAULT 0 COMMENT 'Number of products already sent or delivered.',
      ADD COLUMN `created_date` datetime DEFAULT CURRENT_TIMESTAMP COMMENT 'When was the product was created.',
      ADD COLUMN `modified_date` datetime DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP COMMENT 'When was the product was created or modified or deleted.'
    EOT;
    CRM_Core_DAO::executeQuery($query);

    // Add indexes for the new datetime fields
    $query = <<<EOT
      ALTER TABLE `civicrm_product`
      ADD INDEX `index_created_date` (`created_date`),
      ADD INDEX `index_modified_date` (`modified_date`)
    EOT;
    CRM_Core_DAO::executeQuery($query);

    echo "Added stock management fields to civicrm_product table.\n";
  }
  else {
    echo "civicrm_product stock management fields already exist.\n";
  }

  // Add restock field to civicrm_contribution_product table
  $query = "SHOW COLUMNS FROM `civicrm_contribution_product` LIKE 'restock'";
  $result = CRM_Core_DAO::executeQuery($query);
  $column_exists = $result->N > 0;

  if (!$column_exists) {
    $query = <<<EOT
      ALTER TABLE `civicrm_contribution_product`
      ADD COLUMN `restock` tinyint(4) DEFAULT 0 COMMENT 'Indicates if contribution product restocked'
    EOT;
    CRM_Core_DAO::executeQuery($query);
    echo "Added restock column to civicrm_contribution_product table.\n";
  }
  else {
    echo "civicrm_contribution_product restock column already exists.\n";
  }

  // Add combination_id field to civicrm_contribution_product table
  $query = "SHOW COLUMNS FROM `civicrm_contribution_product` LIKE 'combination_id'";
  $result = CRM_Core_DAO::executeQuery($query);
  $column_exists = $result->N > 0;

  if (!$column_exists) {
    $query = <<<EOT
      ALTER TABLE `civicrm_contribution_product`
      ADD COLUMN `combination_id` int unsigned DEFAULT NULL COMMENT 'ID for product combination'
    EOT;
    CRM_Core_DAO::executeQuery($query);
    echo "Added combination_id column to civicrm_contribution_product table.\n";
  }
  else {
    echo "civicrm_contribution_product combination_id column already exists.\n";
  }
}
