<?php
function _neticrm_update_update_x316() {
  $exists = CRM_Core_DAO::singleValueQuery("SHOW Tables LIKE 'civicrm_contribution_mypay'");
  if (!$exists) {
    $sql = "CREATE TABLE civicrm_contribution_mypay (
     id int unsigned NOT NULL AUTO_INCREMENT COMMENT 'MyPay ID',
     contribution_id int unsigned DEFAULT NULL COMMENT 'Contribuution ID',
     contribution_recur_id int unsigned COMMENT 'Contribuution Recur ID',
     uid varchar(255) COMMENT 'Payment Hub User Order ID',
     uid_key varchar(255) COMMENT 'Validation text by order.',
     expired_date date COMMENT 'Used date time of this coupon track.',
     create_post_data text COMMENT 'The post data when send request.',
     create_result_data text COMMENT 'The result data after sending request.',
     ipn_result_data text COMMENT 'The post data as IPN sending to us.',
     PRIMARY KEY (id),
     INDEX UI_uid(uid),
     CONSTRAINT FK_civicrm_contribution_mypay_contribution_id FOREIGN KEY (contribution_id) REFERENCES civicrm_contribution(id) ON DELETE
     SET NULL,
          CONSTRAINT FK_civicrm_contribution_mypay_contribution_recur_id FOREIGN KEY (contribution_recur_id) REFERENCES civicrm_contribution_recur(id) ON DELETE
     SET NULL
) ENGINE = InnoDB DEFAULT CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci ROW_FORMAT = DYNAMIC;";
    CRM_Core_DAO::executeQuery($sql);
    echo "civicrm_contribution_mypay create success!!\n";
  }
  else {
    echo "civicrm_contribution_mypay is exist.";
  }

  $exists = CRM_Core_DAO::singleValueQuery("SHOW Tables LIKE 'civicrm_contribution_mypay_log'");
  if (!$exists) {
    $sql = "CREATE TABLE civicrm_contribution_mypay_log (
      id int unsigned NOT NULL AUTO_INCREMENT COMMENT 'MyPay Log ID',
      contribution_id int unsigned DEFAULT NULL COMMENT 'Contribuution ID',
      url varchar(255) COMMENT 'Request URL',
      cmd varchar(255) COMMENT 'Request URL',
      date datetime COMMENT 'Request Date Time',
      post_data text COMMENT 'Request Post Data',
      return_data text COMMENT 'Request Return Data',
      PRIMARY KEY (id),
      INDEX UI_url(url),
      INDEX UI_date(date),
      CONSTRAINT FK_civicrm_contribution_mypay_log_contribution_id FOREIGN KEY (contribution_id) REFERENCES civicrm_contribution(id) ON DELETE
      SET NULL
 ) ENGINE = InnoDB DEFAULT CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci ROW_FORMAT = DYNAMIC;";
    CRM_Core_DAO::executeQuery($sql);
    echo "civicrm_contribution_mypay_log create success!!\n";
  }
  else {
    echo "civicrm_contribution_mypay_log is exist.";
  }

  $exists = CRM_Core_DAO::singleValueQuery("SELECT id FROM civicrm_payment_processor_type WHERE NAME = 'MyPay'");
  if (!$exists) {
    $sql = "
    INSERT INTO `civicrm_payment_processor_type` (name, title, description, is_active, is_default, user_name_label, password_label, signature_label, subject_label, class_name, url_site_default, url_api_default, url_recur_default, url_button_default, url_site_test_default, url_api_test_default, url_recur_test_default, url_button_test_default, billing_mode, is_recur )
VALUES ('MyPay', 'MyPay', NULL, 1, 0, 'Store ID', 'Secret', 'In-App Token', '', 'Payment_MyPay', NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 4, 1);";
    CRM_Core_DAO::executeQuery($sql);
    echo "MyPay processor type create success!!\n";
  }
  else {
    echo "payment_processor_type MyPay is exist.";
  }
}
