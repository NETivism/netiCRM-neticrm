<?php

function _neticrm_update_update_x329() {
  civicrm_initialize();

  // Check if civicrm_aiimagegeneration table exists
  $query = "SHOW TABLES LIKE 'civicrm_aiimagegeneration'";
  $result = CRM_Core_DAO::executeQuery($query);
  $table_exists = $result->N > 0;

  if (!$table_exists) {
    $sql = "CREATE TABLE civicrm_aiimagegeneration (
      `id` int unsigned NOT NULL AUTO_INCREMENT COMMENT 'AIImageGeneration ID',
      `aicompletion_id` int unsigned COMMENT 'FK to civicrm_aicompletion',
      `original_prompt` text NOT NULL COMMENT 'Original user input prompt',
      `translated_prompt` text NOT NULL COMMENT 'Translated and optimized prompt',
      `image_style` varchar(255) NOT NULL COMMENT 'Selected image style',
      `image_ratio` varchar(20) NOT NULL COMMENT 'Image aspect ratio (e.g., 16:9, 1:1)',
      `image_path` varchar(255) NOT NULL COMMENT 'Generated image file path',
      `created_date` datetime NOT NULL COMMENT 'Image generation date',
      `status_id` int unsigned NOT NULL COMMENT '1=success, 2=pending, 3=failed',
      `error_message` text COMMENT 'Error message if generation failed',
      PRIMARY KEY (`id`),
      CONSTRAINT `FK_civicrm_aiimagegeneration_aicompletion_id` FOREIGN KEY (`aicompletion_id`) REFERENCES `civicrm_aicompletion` (`id`) ON DELETE SET NULL
    ) ENGINE=InnoDB DEFAULT CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci ROW_FORMAT=DYNAMIC;";

    CRM_Core_DAO::executeQuery($sql);

    echo "Table civicrm_aiimagegeneration created successfully.";
  }
  else {
    echo "Table civicrm_aiimagegeneration already exists, skipping creation.";
  }
}