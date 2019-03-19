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

  $sql = "INSERT INTO `civicrm_payment_processor_type` 
 (name, title, description, is_active, is_default, user_name_label, password_label, signature_label, subject_label, class_name, url_site_default, url_api_default, url_recur_default, url_button_default, url_site_test_default, url_api_test_default, url_recur_test_default, url_button_test_default, billing_mode, is_recur )
VALUES 
('TapPay', '{TapPay Payment}', NULL, 1, 0, 'Merchant ID', '{Partner Key}', '{App ID}', '{App Key}',  'Payment_TapPay', NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,  4, 1);";
  $sql = str_replace("{TapPay Payment}", ts("TapPay Payment"), $sql);
  $sql = str_replace("{Partner Key}", ts("Partner Key"), $sql);
  $sql = str_replace("{App ID}", ts("App ID"), $sql);
  $sql = str_replace("{App Key}", ts("App Key"), $sql);
  CRM_Core_DAO::executeQuery($sql);
}

