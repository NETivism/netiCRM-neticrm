<?php
function _neticrm_update_update_7252() {
  CRM_Core_DAO::executeQuery("UPDATE `civicrm_mailing_component` SET body_html = REPLACE(body_html, '您已取消訂閱：', '您已取消訂閱 '), body_text = REPLACE(body_text, '您已取消訂閱：', '您已取消訂閱 ') WHERE component_type = 'Unsubscribe'");
  CRM_Core_DAO::executeQuery("UPDATE `civicrm_mailing_component` SET body_html = REPLACE(body_html, '您已重新訂閱以下電子報：', '您已重新訂閱 '), body_text = REPLACE(body_text, '您已重新訂閱以下電子報：', '您已重新訂閱 ') WHERE component_type = 'Resubscribe'");
}
