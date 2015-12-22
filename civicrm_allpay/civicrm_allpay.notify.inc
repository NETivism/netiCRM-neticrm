<?php

function civicrm_allpay_notify_generate($cid, $send_mail = FALSE){
  civicrm_initialize();
  $info = civicrm_allpay_notify_info($cid);
  if(!empty($info)){
    $info['display'] = civicrm_allpay_notify_display($info);
    if($info['#type'] == 'BAR'){
      $info['has_receipt'] = 1;
      $info['display'] = civicrm_allpay_notify_display_barcode($info);
    }

    if($send_mail){
      $message = '';
      CRM_Contribute_BAO_Contribution::getInvoice($cid, $info, $message, $send_mail);
    }
    else{
      return $info;
    }
  }
}

function civicrm_allpay_notify_bytrxn($trxn_id){
  $cid = CRM_Core_DAO::singleValueQuery("SELECT id FROM civicrm_contribution WHERE trxn_id = %1", array(1 => array($trxn_id, 'String')));
  return civicrm_allpay_notify_info($cid);
}

function civicrm_allpay_notify_info($cid){
  $data = db_query("SELECT data FROM {civicrm_contribution_allpay} WHERE cid = :cid", array(':cid' => $cid))->fetchField();
  $json = json_decode($data, TRUE);
  if(!empty($json['#info'])){
    $info =& $json['#info'];
    $type = substr($info['PaymentType'], 0, 3);
    $return = array();
    $invoice_link = CRM_Contribute_BAO_Contribution_Utils::invoiceLink($cid);
    switch($type){
      case 'ATM':
        $return['繳費銀行代碼'] = $info['BankCode'];
        $return['繳費虛擬帳號'] = $info['vAccount'];
        $return['繳費期限'] = $info['ExpireDate'];
        break;
      case 'CVS':
        $help = l('» 步驟說明', 'https://www.allpay.com.tw/Service/pay_way_cvpay', array('attributes' => array('target' => '_blank')));
        $return['便利商店繳費代碼'] = $info['PaymentNo'];
        $return['繳費期限'] = $info['ExpireDate'];
        $return[] = $help;
        break;
      case 'BAR':
        $barcodepath = 'https://pay.allpay.com.tw/bank/tcbank/cnt/GenerateBarcode?barcode=';
        $barcode = array($info['Barcode1'], $info['Barcode2'], $info['Barcode3']);
        foreach($barcode as $k => $b){
          $return['#barcode'.$k] = '<div><img src="'.$barcodepath.$b.'" alt="'.$b.'" /></div>';
        }
        $return['繳費單'] = '<a href="'.$invoice_link.'" target="_blank">» 列印繳費單</a>';
        $return['繳費期限'] = $info['ExpireDate'];
        break;
    }
    if(!empty($return)){
      $return['#type'] = $type;
      $return['#expire'] = $info['ExpireDate'];
      $return['#invoice'] = $invoice_link;
      return $return;
    }
  }

  // error fallback
  return array();
}

function civicrm_allpay_notify_display($info, $payment_instrument_id = NULL){
  $instruments = CRM_Contribute_PseudoConstant::paymentInstrument();
  foreach($info as $label => $value){
    if(preg_match('/^#/', $label)){
      continue;
    }
    elseif(is_numeric($label)){
      $output .= '<dd>'.' <span>'.$value.'</span></dd>';
    }
    else{
      $output .= '<dd><label>'.$label.':</label>'.' <span>'.$value.'</span></dd>';
    }
  }
  $payment_instrument = !empty($payment_instrument_id) ? '<strong>'.$instruments[$payment_instrument_id].'</strong>' : '';
  $output = $payment_instrument.'<dl>'.$output.'</dl>';
  return $output;
}

function civicrm_allpay_notify_display_barcode($info){
  $output = '<div style="border:1px solid #000;padding:10px;margin:10px;float:right;width:200px;height:175px;color:#777;">代收店舖章</div>';
  foreach($info as $label => $v){
    if(preg_match('/^#barcode/', $label)){
      $output .= '<div>'.$v.'</div>';
    }
  }
  $output .= '<p><label>繳費期限:</label> <span>'.$info['#expire'].'</span></p>';
  return $output;
}
