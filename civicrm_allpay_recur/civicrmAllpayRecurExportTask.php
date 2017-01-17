<?php
class civicrmAllpayRecurExportTask extends CRM_Contact_Form_Task {
  public function buildQuickForm(){
    $this->add('text', "credit_item", '刷卡項目', array('size' => 30, 'maxlength' => 16), TRUE);
    $this->addElement('password', 'allpayexportpasswd', '資料匯出密碼');
    $this->addDefaultButtons(t('Export'), 'done');
    $this->_defaults['credit_item'] = '定期定額';
    $this->addFormRule(array('civicrmAllpayRecurExportTask', 'formRule'));
  }

  public function formRule($fields){
    $errors = array();
    $pass = variable_get('allpayexportpasswd', '');
    if(!empty($pass)) {
      if($pass !== strtolower(md5($fields['allpayexportpasswd']))) {
        $errors['allpayexportpasswd'] = '您輸入的匯出密碼不正確，資料無法匯出';
      }
    }
    return $errors;
  }

  public function postProcess(){
    $params = $this->controller->exportValues();
    $fields = variable_get('civicrm_allpay_recur_fid', array());
    $gid = variable_get('civicrm_allpay_recur_gid', 0);
    $export_header = _civicrm_allpay_export_header();
    $lookup = array_intersect_key($fields, $export_header);
    $query = CRM_Core_DAO::executeQuery("SELECT id, column_name FROM civicrm_custom_field WHERE id IN (".implode(',', $lookup).")");
    $column_name = array();
    while($query->fetch()){
      $index = array_search($query->id, $lookup);
      $column_name[$index] = $query->column_name;
    }
    $query->free();
    $table_name = CRM_Core_DAO::singleValueQuery("SELECT table_name FROM civicrm_custom_group WHERE id = $gid");
    $sql = "SELECT 
    '{$params['credit_item']}' as item, '0' as installment,
    t.{$column_name['allpay_amount']} as allpay_amount,
    CONCAT('[', entity_id, ']') as name,
    t.{$column_name['allpay_cardno']} as allpay_cardno,
    t.{$column_name['allpay_duedate']} as allpay_duedate,
    '' as digit
    FROM {$table_name} t
    INNER JOIN civicrm_contact c ON c.id = t.entity_id
    WHERE t.entity_id IN (".implode(',', $this->_contactIds).")";
    $query = CRM_Core_DAO::executeQuery($sql);
    $rand = substr(md5(microtime(TRUE)), 0, 4);
    $filename = '/tmp/'."allpay_recur_" . $_SERVER['REQUEST_TIME'] . $rand;
    $fp = fopen($filename, 'w');
    fputcsv($fp, $export_header);
    while($query->fetch()){
      $temp = (array) $query;
      $row = array_intersect_key($temp, $export_header);
      $row['allpay_cardno'] = _civicrm_allpay_recur_decrypt_cardno($row['allpay_cardno'], $mask = FALSE);
      $row['allpay_duedate'] = _civicrm_allpay_recur_format_duedate($row['allpay_duedate']);
      fputcsv($fp, $row);
    }
    fclose($fp);
    CRM_Core_Report_Excel::writeExcelFile($filename);
  }
}
