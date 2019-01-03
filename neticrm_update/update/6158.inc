<?php

function _neticrm_update_update_6158() {
  $sql = "
CREATE TABLE IF NOT EXISTS civicrm_coupon (
  id int unsigned NOT NULL AUTO_INCREMENT  COMMENT 'ID for internal usage',
  start_date datetime    COMMENT 'Start date of this coupon.',
  end_date datetime    COMMENT 'End date of this coupon.',
  code varchar(255) NOT NULL   COMMENT 'Code of this coupon',
  coupon_type varchar(32) NOT NULL   COMMENT 'Coupon type shoule be one of percent and monetary.',
  discount int unsigned NOT NULL   COMMENT 'Amount of discount, actual or percentage',
  minimal_amount int unsigned    COMMENT 'Minimal amount before apply discount.',
  count_max int unsigned NOT NULL   COMMENT 'Max number of times this code can be used.',
  description varchar(255) NOT NULL   COMMENT 'Discount description',
  is_active tinyint    COMMENT 'Is this discount active?'
,
  PRIMARY KEY ( id ),
  INDEX index_start_date(start_date),
  INDEX index_end_date(end_date),
  INDEX index_code(code)
)  ENGINE=InnoDB DEFAULT CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci ROW_FORMAT=DYNAMIC
";
  CRM_Core_DAO::executeQuery($sql);

  $sql = "
CREATE TABLE IF NOT EXISTS civicrm_coupon_entity (
  id int unsigned NOT NULL AUTO_INCREMENT  COMMENT 'ID for internal usage',
  coupon_id int unsigned NOT NULL   COMMENT 'FK to coupon id',
  entity_table varchar(64)    COMMENT 'Entity table like civicrm_event or civicrm_price_option.',
  entity_id int unsigned    COMMENT 'Entity id like event_id or price_option_id'
,
  PRIMARY KEY ( id ),
  INDEX index_entity_table_id(entity_table, entity_id),
  CONSTRAINT FK_civicrm_coupon_entity_coupon_id FOREIGN KEY (coupon_id) REFERENCES civicrm_coupon(id) ON DELETE CASCADE
)  ENGINE=InnoDB DEFAULT CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci ROW_FORMAT=DYNAMIC
";
  CRM_Core_DAO::executeQuery($sql);

  $sql = "
CREATE TABLE IF NOT EXISTS civicrm_coupon_track (
  id int unsigned NOT NULL AUTO_INCREMENT  COMMENT 'ID for internal usage',
  coupon_id int unsigned    COMMENT 'FK to coupon id',
  contact_id int unsigned   DEFAULT NULL COMMENT 'FK to contact id',
  contribution_id int unsigned   DEFAULT NULL COMMENT 'FK to contribution id, means used.',
  used_date datetime    COMMENT 'Used date time of this coupon track.' 
,
  PRIMARY KEY ( id ),
  INDEX index_used_date(used_date),      
  CONSTRAINT FK_civicrm_coupon_track_coupon_id FOREIGN KEY (coupon_id) REFERENCES civicrm_coupon(id) ON DELETE CASCADE,      
  CONSTRAINT FK_civicrm_coupon_track_contact_id FOREIGN KEY (contact_id) REFERENCES civicrm_contact(id) ON DELETE SET NULL,      
  CONSTRAINT FK_civicrm_coupon_track_contribution_id FOREIGN KEY (contribution_id) REFERENCES civicrm_contribution(id) ON DELETE SET NULL  
)  ENGINE=InnoDB DEFAULT CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci ROW_FORMAT=DYNAMIC
";
  CRM_Core_DAO::executeQuery($sql);
}
