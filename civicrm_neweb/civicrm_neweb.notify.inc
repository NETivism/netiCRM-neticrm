<?php

function civicrm_neweb_notify_generate($cid, $send_mail = FALSE){
  $info = civicrm_neweb_notify_info($cid);
  if(!empty($info)){
    $info['display'] = civicrm_neweb_notify_display($info);
    if($info['#type'] == 'BAR'){
      $info['has_receipt'] = 1;
      $info['display'] = civicrm_neweb_notify_display_barcode($info);
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

function civicrm_neweb_notify_bytrxn($trxn_id){
  $cid = CRM_Core_DAO::singleValueQuery("SELECT id FROM civicrm_contribution WHERE trxn_id = %1", array(1 => array($trxn_id, 'String')));
  return civicrm_neweb_notify_info($cid);
}

function civicrm_neweb_notify_info($cid){
  $sql = "SELECT payment_processor_id AS pid,is_test FROM civicrm_contribution WHERE id = $cid";
  $dao = CRM_Core_DAO::executeQuery($sql);
  $dao->fetch();
  if($dao->pid){
    $p = new CRM_Core_BAO_PaymentProcessor();
    $payment_processor = $p->getPayment($dao->pid,$dao->is_test);
  }

  $sql = "SELECT return_data, post_data FROM civicrm_contribution_neweb WHERE cid = $cid";
  $dao = CRM_Core_DAO::executeQuery($sql);
  if($dao->fetch()){
    $data = $dao->return_data;
    $post = $dao->post_data;
  }
  $post = json_decode($post, TRUE);
  $info = json_decode($data, TRUE);
  if(!empty($info) && $info['rc'] == 0){
    $invoice_link = CRM_Contribute_BAO_Contribution_Utils::invoiceLink($cid);
    if(!empty($info['paycode'])){
      // 超商代碼
      $return['便利商店繳費代碼'] = $info['paycode'];
    }
    elseif(!empty($info['bankid'])){
      $return['轉入銀行代碼'] = $info['bankid'];
      $return['虛擬帳號'] = $info['virtualaccount'];
    }
    elseif(!empty($info['barcode1'])){
      $barcodepath = $payment_processor['url_api'].'BarcodeGenerator?barcode=';
      $barcode = array($info['barcode1'], $info['barcode2'], $info['barcode3']);
      foreach($barcode as $k => $b){
        $return['#barcode'.$k] = '<div><img src="'.$barcodepath.$b.'" alt="'.$b.'" height="40px" width="'.(strlen($b)*15+30).'px" /></div><div style="font-size:13px;padding-bottom:4px;">'.$b.'</div>';
        // $return['條碼'.($k+1)] = $return['#barcode'.$k];
      }
      $return['繳費單'] = '<a href="'.$invoice_link.'" target="_blank">» 列印繳費單</a>';
      $type = 'BAR';
    }

    $return['繳費期限'] = date('Y 年 m 月 d 日',strtotime((empty($info['duedate']))?$post['duedate']:$info['duedate']));
    

    if(!empty($return)){
      $return['#type'] = $type;
      $return['#expire'] = $return['繳費期限'];
      $return['#invoice'] = $invoice_link;
      return $return;
    }
  }

  // error fallback
  return array();
}

function civicrm_neweb_notify_display($info, $payment_instrument_id = NULL){
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

function civicrm_neweb_notify_display_barcode($info){
  $output = '<div style="border:1px solid #000;padding:10px;margin:10px;float:right;width:200px;height:175px;color:#777;">代收店舖章</div>';
  foreach($info as $label => $v){
    if(preg_match('/^#barcode/', $label)){
      $output .= '<div style="text-align:center;">'.$v.'</div>';
    }
  }
  $output .= '<p><label>繳費期限:</label> <span>'.$info['#expire'].'</span></p>';
  return $output;
}
