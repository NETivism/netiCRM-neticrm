<?php
function _neticrm_update_update_7254() {
  $exists = CRM_Core_DAO::singleValueQuery("SHOW Tables LIKE 'civicrm_contribution_linepay'");
  if (!$exists) {
    $sql = "
CREATE TABLE civicrm_contribution_linepay (
  id int unsigned NOT NULL AUTO_INCREMENT  COMMENT 'LinePay ID',
  trxn_id varchar(255)    COMMENT 'trxn_id from contribution',
  transaction_id varchar(255)    COMMENT 'transaction id from linepay',
  query text   DEFAULT NULL COMMENT 'response of /payments',
  request text   DEFAULT NULL COMMENT 'response of /payments/request',
  confirm text   DEFAULT NULL COMMENT 'response of /payments/{transactionId}/confirm',
  refund text   DEFAULT NULL COMMENT 'response of /payments/{transactionId}/refund',
  authorization text   DEFAULT NULL COMMENT 'response of /payments/authorization',
  capture text   DEFAULT NULL COMMENT 'response of /payments/authorizations/{transactionId}/capture',
  void text   DEFAULT NULL COMMENT 'response of /payments/authorizations/{transactionId}/void',
  recurring_payment text   DEFAULT NULL COMMENT 'response of /payments/preapprovedPay/{regKey}/payment',
  recurring_check text   DEFAULT NULL COMMENT 'response of /payments/preapprovedPay/{regKey}/check',
  recurring_expire text   DEFAULT NULL COMMENT 'response of /payments/preapprovedPay/{regKey}/expire',
  PRIMARY KEY (id),
  INDEX UI_contrib_trxn_id(trxn_id),
  INDEX UI_transaction_id(transaction_id)
) ENGINE=InnoDB DEFAULT CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci ROW_FORMAT=DYNAMIC;
";
    CRM_Core_DAO::executeQuery($sql);
  }
}
