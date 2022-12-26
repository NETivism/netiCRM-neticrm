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
  $data = db_result(db_query("SELECT data FROM {civicrm_contribution_spgateway} WHERE cid = %d", $cid));
  $json = _civicrm_spgateway_post_decode($data);
  $type = $json['PaymentType'];
  $return = array();
  $invoice_link = CRM_Contribute_BAO_Contribution_Utils::invoiceLink($cid);
  switch($type){
    case 'VACC':
      $return[ts('Bank code')] = $json['BankCode'];
      $return[ts('Virtual account number')] = $json['CodeNo'];
      $return[ts('Payment deadline')] = $json['ExpireDate'].' '.$json['ExpireTime'];
      break;
    case 'CVS':
      $help = l('» '.ts('Payment Instructions'), 'https://www.newebpay.com/website/Page/content/cvs_intro', array('attributes' => array('target' => '_blank')));
      $return[ts('Convenience Store Payment Code')] = $json['CodeNo'];
      $return[ts('Payment deadline')] = $json['ExpireDate'].' '.$json['ExpireTime'];
      $return[] = $help;
      break;
    case 'BARCODE':
      $barcodepath = SPGATEWAY_REAL_DOMAIN.'/API/barcode_display/get_barcode_img?barcode_text=';
      $barcode = array($json['Barcode_1'], $json['Barcode_2'], $json['Barcode_3']);
      foreach($barcode as $k => $b){
        $return['#barcode'.$k] = '<div><img src="'.$barcodepath.$b.'" alt="'.$b.'" /></div>';
      }
      $return[ts('Payment slip')] = '<a href="'.$invoice_link.'" target="_blank">» '.ts('Print payment slip').'</a>';
      $return[ts('Payment deadline')] = $json['ExpireDate'].' '.$json['ExpireTime'];
      break;
  }
  if(!empty($return)){
    $return['#type'] = $type;
    $return['#expire'] = $json['ExpireDate'].' '.$json['ExpireTime'];
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
  $output .= '<p><label>'.ts('Payment deadline').':</label> <span>'.$info['#expire'].'</span></p>';
  $output .= '<p><span style="font-size: 9px;">'.ts('Note: Payment confirmation takes 3-4 working days.').'</span></p>';
  return $output;
}
