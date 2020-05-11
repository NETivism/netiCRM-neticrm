<?php
function _neticrm_update_update_6177() {
  $exists = CRM_Core_DAO::singleValueQuery("SHOW Tables LIKE 'civicrm_contribution_taiwanach'");
  if (!$exists) {
    $sql = "CREATE TABLE civicrm_contribution_taiwanach (
      id int unsigned NOT NULL AUTO_INCREMENT  COMMENT 'Taiwan ACH  ID',
      contact_id int unsigned NOT NULL   COMMENT 'FK to Contact ID',
      contribution_recur_id int unsigned NOT NULL   COMMENT 'Conditional foreign key to civicrm_contribution_recur id. Each contribution made in connection with a recurring contribution carries a foreign key to the recurring contribution record. This assumes we can track these processor initiated events.',
      contribution_page_id int unsigned    COMMENT 'The Contribution Page which triggered this contribution',
      payment_type varchar(32)    COMMENT 'Payment Type of ACH.',
      bank_code varchar(32)    COMMENT 'Bank Code of ACH Account.',
      postoffice_acc_type varchar(32)    COMMENT 'Post Office Account Type of ACH Account.',
      bank_branch varchar(32)    COMMENT 'Bank Branch of ACH Account.',
      bank_account varchar(32)    COMMENT 'Bank Account of ACH Account.',
      identifier_number varchar(128)    COMMENT 'Identifier Number of ACH Account.',
      stamp_verification int unsigned    COMMENT 'Stamp Verification Status',
      data text    COMMENT 'Some Custom Record Data' ,
     PRIMARY KEY ( id ) ,
      CONSTRAINT FK_civicrm_contribution_taiwanach_contact_id FOREIGN KEY (contact_id) REFERENCES civicrm_contact(id) ON DELETE CASCADE,      
      CONSTRAINT FK_civicrm_contribution_taiwanach_contribution_recur_id FOREIGN KEY (contribution_recur_id) REFERENCES civicrm_contribution_recur(id) ON DELETE CASCADE,      
      CONSTRAINT FK_civicrm_contribution_taiwanach_contribution_page_id FOREIGN KEY (contribution_page_id) REFERENCES civicrm_contribution_page(id) ON DELETE SET NULL  
 )  ENGINE=InnoDB DEFAULT CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci ROW_FORMAT=DYNAMIC  ;
 ";
    CRM_Core_DAO::executeQuery($sql);
  }
  else {
    echo "civicrm_contribution_taiwanach is exist.";
  }

  $exists = CRM_Core_DAO::singleValueQuery("SELECT name FROM civicrm_payment_processor_type WHERE name = 'TaiwanACH'");
  if (!$exists) {
    $sql = "INSERT INTO `civicrm_payment_processor_type` 
   (name, title, description, is_active, is_default, user_name_label, password_label, signature_label, subject_label, class_name, url_site_default, url_api_default, url_recur_default, url_button_default, url_site_test_default, url_api_test_default, url_recur_test_default, url_button_test_default, billing_mode, is_recur )
  VALUES 
  ('TaiwanACH', '{Taiwan ACH Payment}', NULL, 1, 0, '{Bank Account Number}', '{Sic Code}', '{Bank Identification Number}', '{Post Delegate Institution Code}',  'Payment_TaiwanACH', NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,  1, 0);";
    $sql = str_replace("{Taiwan ACH Payment}", ts("Taiwan ACH Payment"), $sql);
    $sql = str_replace("{Bank Account Number}", ts("Bank Account Number"), $sql);
    $sql = str_replace("{Sic Code}", ts("Sic Code"), $sql);
    $sql = str_replace("{Bank Identification Number}", ts("Bank Identification Number"), $sql);
    $sql = str_replace("{Post Delegate Institution Code}", ts("Post Delegate Institution Code"), $sql);
    CRM_Core_DAO::executeQuery($sql);
  }
  else {
    echo "TaiwanACH is exist in civicrm_payment_processor_type.";
  }
}

