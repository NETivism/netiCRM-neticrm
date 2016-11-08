<?php

/**
 * Form builder; Configure invpay2go secret settings
 *
 * @ingroup forms
 * @see system_settings_form()
 */
function invpay2go_admin_settings() {
  $form = array();
  $db = db_select('invpay2go', 'inv')
    ->fields('inv')
    ->orderBy('pid', 'ASC')
    ->execute();
  $form['#tree'] = TRUE;
  while($payment = $db->fetchAssoc()) {
    $pid = $payment['pid'];
    $container = _invpay2go_admin_settings_form($pid, $payment);
    $form['container'][$pid] = $container;
  }
  $pid++;
  $form['container'][$pid] = _invpay2go_admin_settings_form($pid, array());
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => 'Submit',
  );
  return $form;
}

function invpay2go_admin_settings_submit($form, &$form_state) {
  $settings = db_select('invpay2go', 'inv')
    ->fields('inv')
    ->execute()
    ->fetchAll();
  $exists = array();
  foreach($settings as $e){
    $exists[$e->pid] = (array) $e;
  }

  foreach ($form_state['values']['container'] as $pid => $invoice_setting) {
    if (empty($invoice_setting)) {
      continue;
    }
    if (isset($exists[$pid]) && !empty($exists[$pid])) {
      if (!empty($invoice_setting['delete'])) {
        db_delete('invpay2go')
          ->condition('pid', $pid)
          ->execute();
      }
      else{
        $fields = array(
          'merchantid' => $invoice_setting['merchantid'],
          'hashkey' => $invoice_setting['hashkey'],
          'hashiv' => $invoice_setting['hashiv'],
          'merchantid_test' => $invoice_setting['merchantid_test'],
          'hashkey_test' => $invoice_setting['hashkey_test'],
          'hashiv_test' => $invoice_setting['hashiv_test'],
        );
        db_update('invpay2go')
          ->fields($fields)
          ->condition('pid', $pid, '=')
          ->execute();
      }
    }
    else{
      if(!empty($invoice_setting['hashkey']) || !empty($invoice_setting['hashiv']) || !empty($invoice_setting['hashkey_test']) || !empty($invoice_setting['hashiv_test']))
      db_insert('invpay2go')
        ->fields(array(
          'merchantid' => $invoice_setting['merchantid'],
          'hashkey' => $invoice_setting['hashkey'],
          'hashiv' => $invoice_setting['hashiv'],
          'merchantid_test' => $invoice_setting['merchantid_test'],
          'hashkey_test' => $invoice_setting['hashkey_test'],
          'hashiv_test' => $invoice_setting['hashiv_test'],
        ))
        ->execute();
    }
  }
}

function _invpay2go_admin_settings_form($pid, $default){
  $form = array();
  $form = array(
    '#type' => 'fieldset',
    '#title' => 'Pay2go Settings'. ': '. $pid,
  );
  $form['merchantid'] = array(
    '#type' => 'textfield',
    '#title' => 'Merchant ID',
    '#default_value' => $default['merchantid'],
    '#size' => 20,
  );
  $form['hashkey'] = array(
    '#type' => 'textfield',
    '#title' => 'Hash Key',
    '#default_value' => $default['hashkey'],
    '#size' => 20,
  );
  $form['hashiv'] = array(
    '#type' => 'textfield',
    '#title' => 'Hash IV',
    '#default_value' => $default['hashiv'],
    '#size' => 20,
  );
  $form['merchantid_test'] = array(
    '#type' => 'textfield',
    '#title' => 'Test Merchant ID',
    '#default_value' => $default['merchantid_test'],
    '#size' => 20,
  );
  $form['hashkey_test'] = array(
    '#type' => 'textfield',
    '#title' => 'Test Hash Key',
    '#default_value' => $default['hashkey_test'],
    '#size' => 20,
  );
  $form['hashiv_test'] = array(
    '#type' => 'textfield',
    '#title' => 'Test Hash IV',
    '#default_value' => $default['hashiv_test'],
    '#size' => 20,
  );
  $form['delete'] = array(
    '#type' => 'checkbox',
    '#title' => 'Delete',
    '#default_value' => 0,
  );
  return $form;
}
