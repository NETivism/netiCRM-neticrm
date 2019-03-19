<?php
function _neticrm_update_update_7260() {
  $sql = "CREATE TABLE IF NOT EXISTS civicrm_contribution_tappay (
       id int unsigned NOT NULL AUTO_INCREMENT  COMMENT 'TapPay ID',
       contribution_id int unsigned   DEFAULT NULL COMMENT 'Contribuution ID',
       contribution_recur_id int unsigned    COMMENT 'Contribuution Recur ID',
       order_number varchar(255)    COMMENT 'Used in parameters of transact. Same as contribution trxn id.',
       card_token varchar(255)    COMMENT 'Card Token',
       card_key varchar(255)    COMMENT 'Card Key',
       rec_trade_id varchar(255)    COMMENT 'Record Trade ID',
       bin_code varchar(32)    COMMENT 'The first 6 words of card number.',
       last_four varchar(32)    COMMENT 'The last four words of card number.',
       expiry_date date    COMMENT 'Used date time of this coupon track.',
       data text    COMMENT 'Newest return data.' ,
      PRIMARY KEY ( id )
      ,     INDEX tappay_order_number(
          order_number  ),     INDEX UI_rec_trade_id(
          rec_trade_id),      
       CONSTRAINT FK_civicrm_contribution_tappay_contribution_id FOREIGN KEY (contribution_id) REFERENCES civicrm_contribution(id) ON DELETE SET NULL,      
       CONSTRAINT FK_civicrm_contribution_tappay_contribution_recur_id FOREIGN KEY (contribution_recur_id) REFERENCES civicrm_contribution_recur(id) ON DELETE SET NULL  
  )  ENGINE=InnoDB DEFAULT CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci ROW_FORMAT=DYNAMIC  ;";
  CRM_Core_DAO::executeQuery($sql);

  $sql = "CREATE TABLE IF NOT EXISTS civicrm_contribution_tappay_log (
       id int unsigned NOT NULL AUTO_INCREMENT  COMMENT 'TapPay Log ID',
       contribution_id int unsigned   DEFAULT NULL COMMENT 'Contribuution ID',
       url varchar(255)    COMMENT 'Request URL',
       date datetime    COMMENT 'Request Date Time',
       post_data text    COMMENT 'Request Post Data',
       return_data text    COMMENT 'Request Return Data' ,
      PRIMARY KEY ( id ),      
       CONSTRAINT FK_civicrm_contribution_tappay_log_contribution_id FOREIGN KEY (contribution_id) REFERENCES civicrm_contribution(id) ON DELETE SET NULL  
  )  ENGINE=InnoDB DEFAULT CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci ROW_FORMAT=DYNAMIC  ;";
  CRM_Core_DAO::executeQuery($sql);
}