<?php

function civicrm_spgateway_notify_generate($cid, $send_mail = FALSE){
  civicrm_initialize();
  $info = civicrm_spgateway_notify_info($cid);
  if(!empty($info)){
    $info['display'] = civicrm_spgateway_notify_display($info);
    if($info['#type'] == 'BARCODE'){
      $info['has_receipt'] = 1;
      $info['display'] = civicrm_spgateway_notify_display_barcode($info);
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

function civicrm_spgateway_notify_bytrxn($trxn_id){
  $cid = CRM_Core_DAO::singleValueQuery("SELECT id FROM civicrm_contribution WHERE trxn_id = %1", array(1 => array($trxn_id, 'String')));
  return civicrm_spgateway_notify_info($cid);
}

function civicrm_spgateway_notify_info($cid){
  $data = db_query("SELECT data FROM {civicrm_contribution_spgateway} WHERE cid = :cid", array(':cid' => $cid))->fetchField();
  $json = _civicrm_spgateway_post_decode($data);
  $type = $json['PaymentType'];
  $return = array();
  $invoice_link = CRM_Contribute_BAO_Contribution_Utils::invoiceLink($cid);
  switch($type){
    case 'VACC':
      $return['繳費銀行代碼'] = $json['BankCode'];
      $return['繳費虛擬帳號'] = $json['CodeNo'];
      $return['繳費期限'] = $json['ExpireDate'];
      break;
    case 'CVS':
      $help = l('» 步驟說明', 'https://www.spgateway.com/info/site_description/transation_convenience', array('attributes' => array('target' => '_blank')));
      $return['便利商店繳費代碼'] = $json['CodeNo'];
      $return['繳費期限'] = $json['ExpireDate'];
      $return[] = $help;
      break;
    case 'BARCODE':
      $barcodepath = 'https://core.spgateway.com/API/barcode_display/get_barcode_img?barcode_text=';
      $barcode = array($json['Barcode_1'], $json['Barcode_2'], $json['Barcode_3']);
      foreach($barcode as $k => $b){
        $return['#barcode'.$k] = '<div><img src="'.$barcodepath.$b.'" alt="'.$b.'" /></div>';
      }
      $return['繳費單'] = '<a href="'.$invoice_link.'" target="_blank">» 列印繳費單</a>';
      $return['繳費期限'] = $json['ExpireDate'];
      break;
  }
  if(!empty($return)){
    $return['#type'] = $type;
    $return['#expire'] = $json['ExpireDate'];
    $return['#invoice'] = $invoice_link;
    return $return;
  }

  // error fallback
  return array();
}

function civicrm_spgateway_notify_display($info, $payment_instrument_id = NULL){
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

function civicrm_spgateway_notify_display_barcode($info){
  $output = '<div style="border:1px solid #000;padding:10px;margin:10px;float:right;width:200px;height:175px;color:#777;">代收店舖章</div>';
  foreach($info as $label => $v){
    if(preg_match('/^#barcode/', $label)){
      $output .= '<div>'.$v.'</div>';
    }
  }
  $output .= '<p><label>繳費期限:</label> <span>'.$info['#expire'].'</span></p>';
  return $output;
}
