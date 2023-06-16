<?php
function _neticrm_update_update_7311() {
  $exists = CRM_Core_DAO::singleValueQuery("SHOW Tables LIKE 'civicrm_aicompletion'");
  if (!$exists) {
    $sql = "CREATE TABLE civicrm_aicompletion (
      id int unsigned NOT NULL AUTO_INCREMENT  COMMENT 'AICompletion ID',
      contact_id int unsigned    COMMENT 'FK to civicrm_contact',
      is_template tinyint NOT NULL  DEFAULT 0 COMMENT 'Set to 1 to indicate that this AI completion is saved as a template.',
      template_title varchar(128)    COMMENT 'Save as template title.',
      tone_style varchar(64) NOT NULL   COMMENT 'AI tone style',
      ai_role varchar(64) NOT NULL   COMMENT 'AI role',
      context text NOT NULL   COMMENT 'AI context',
      output_text text NOT NULL   COMMENT 'text that AI return',
      is_share_with_others tinyint NOT NULL  DEFAULT 0 COMMENT 'Mark 1 when this context allow to share with other',
      created_date datetime NOT NULL   COMMENT 'the day create AI completion',
      prompt text NOT NULL   COMMENT 'context that send to AI',
      component varchar(64) NOT NULL   COMMENT 'Name of the function where the referenced item is used in.',
      field varchar(64) NOT NULL   COMMENT 'Additional data field to be used in AI completion.',
      temperature double NOT NULL   COMMENT 'If the number is higher the more creative.',
      post_data text NOT NULL   COMMENT 'Data to be sent as a POST request to the AI.',
      return_data text NOT NULL   COMMENT 'Data returned from the AI server.',
      prompt_token int unsigned NOT NULL   COMMENT 'the token for prompt',
      completion_token int unsigned NOT NULL   COMMENT 'the token for completion',
      status_id int unsigned NOT NULL   COMMENT '1=completed, 2=pending, 3=cancel, 4=failed',
      PRIMARY KEY ( id ),
      CONSTRAINT FK_civicrm_aicompletion_contact_id FOREIGN KEY (contact_id) REFERENCES civicrm_contact(id) ON DELETE SET NULL
      )  ENGINE=InnoDB DEFAULT CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci ROW_FORMAT=DYNAMIC  ;";
    CRM_Core_DAO::executeQuery($sql);
  }
  else {
    echo "civicrm_aicompletion is exist.";
  }
}

