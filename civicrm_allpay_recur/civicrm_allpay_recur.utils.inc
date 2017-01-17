<?php

function _civicrm_allpay_recur_custom_group($name, $title, $extends){
  $params = array(
    'title' => $title,
    'version' => 3,
  );
  $custom_group = civicrm_api('custom_group', 'get', $params);
  if(empty($custom_group['id'])){
    $params = array(
      'title' => $title,
      'name' => $name,
      'extends' => $extends,
      'weight' => 4,
      'collapse_display' => 0,
      'style' => 'Inline',
      'help_pre' => '',
      'help_post' => '',
      'is_active' => 1,
      'version' => 3,
    );
    $custom_group = civicrm_api( 'custom_group','create',$params );
  }
  return $custom_group;
}

function _civicrm_allpay_recur_custom_field($name, $title, $data_type, $html_type, $group_id, $default = NULL, $option_values = NULL){
  static $field_weight;
  $field_weight++;
  $params = array(
    'label' => $title,
    'version' => 3,
  );
  $field = civicrm_api('custom_field', 'get', $params);
  if(empty($field['id'])){
    $params = array(
      'custom_group_id' => $group_id,
      'name' => $name,
      'label' => $title,
      'data_type' => $data_type,
      'html_type' => $html_type,
      'default_value' => '',
      'weight' => $field_weight,
      'is_required' => 0,
      'is_searchable' => 1,
      'is_active' => 1,
      'version' => 3,
    );
    if(!is_null($default)){
      $params['default_value'] = $default;
    }
    if(!is_null($option_values)){
      $params['option_values'] = $option_values;
    }
    $field = civicrm_api('custom_field', 'create', $params);
  }
  return $field;
}

function _civicrm_allpay_recur_create_recur($contact_id, $recur_params){
  // make sure we have only one recurring contribution for allpay manually
  $params = array(
    'version' => 3,
    'contact_id' => $contact_id,
  );
  $result = civicrm_api('ContributionRecur', 'get', $params);
  $recur_id = NULL;
  if(!empty($result['count'])){
    foreach($result['values'] as $rid => $r){
      if(!strstr($r['invoice_id'], 'civicrm_allpay_recur')){
        continue;
      }
      if($r['contribution_status_id'] == 5){
        $recur_id = $r['id'];
        break;
      }
    }
  }
  if(empty($recur_id)){
    $invoice_id = substr(md5(microtime()), 0, 8);
    $default_params = array(
      'version' => 3,
      'contact_id' => $contact_id,
      'invoice_id' => 'civicrm_allpay_recur_'.$contact_id.'_'.$invoice_id,
      'create_date' => date('YmdHis'),
      'start_date' => date('YmdHis'),
      'modified_date' => date('YmdHis'),
      'contribution_status_id' => 5,
      'is_test' => 0,
      'amount' => $amount,
      'currency' => 'TWD',
      'frequency_unit' => 'month',
      'frequency_interval' => 1,
      'installments' => 'null',
    );
    $recur_params += $default_params; 
    $recur = civicrm_api('ContributionRecur', 'create', $recur_params);
    if(!empty($recur['count'])){
      $recur_id = $recur['id'];
    }
  }
  return $recur_id;
}

function _civicrm_allpay_recur_create_contribution($contact_id, $contrib_params){
  if(isset($contrib_params['invoice_id'])){
    $params = array(
      'version' => 3,
      'contact_id' => $contact_id,
      'invoice_id' => $contrib_params['invoice_id'],
    );
    $duplicate = civicrm_api('Contribution', 'get', $params);
    if(!empty($duplicate['id'])){
      $duplicate['duplicate'] = TRUE;
      return $duplicate;
    }
  }
  $default_params = array(
    'version' => 3,
    'contact_id' => $contact_id,
    'contribution_status_id' => 2,
    'is_test' => 0,
  );
  $contrib_params += $default_params; 
  $contrib = civicrm_api('Contribution', 'create', $contrib_params);
  if(!empty($contrib['id'])){
    return $contrib['id'];
  }
  else{
    return $contrib;
  }
}

function _civicrm_allpay_recur_create_note($contact_id, $contribution_id, $note){
  $note_id = NULL;
  $note = date("Y/m/d H:i:s"). ts("Transaction record").": \n".$note."\n===============================\n";
  $note_exists = CRM_Core_BAO_Note::getNote($contribution_id, 'civicrm_contribution');
  if(count($note_exists)){
    $note_params = array('id' => reset(array_keys($note_exists)));
    $note = $note . reset($note_exists);
  }
  
  $noteParams = array(
    'entity_table'  => 'civicrm_contribution',
    'note'          => $note,
    'entity_id'     => $contribution_id,
    'contact_id'    => $contact_id,
    'modified_date' => date('YmdHis')
  );
  CRM_Core_BAO_Note::add($noteParams, $note_params);
}
