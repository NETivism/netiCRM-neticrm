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
  $form['container'][$pid]['#collapsible'] = TRUE;
  $form['container'][$pid]['#collapsed'] = TRUE;
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
          'title' => $invoice_setting['title'],
          'serial' => $invoice_setting['serial'],
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
          'title' => $invoice_setting['title'],
          'serial' => $invoice_setting['serial'],
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
    '#title' => t('Pay2go Settings'). ': '. $pid,
    '#collapsible' => TRUE,
  );
  $form['title'] = array(
    '#type' => 'textfield',
    '#title' => t('Invoice Title'),
    '#default_value' => $default['title'],
    '#size' => 64,
  );
  $form['serial'] = array(
    '#type' => 'textfield',
    '#title' => t('Serial Number'),
    '#default_value' => $default['serial'],
    '#size' => 20,
  );
  $form['merchantid'] = array(
    '#type' => 'textfield',
    '#title' => t('Merchant ID'),
    '#default_value' => $default['merchantid'],
    '#size' => 20,
  );
  $form['hashkey'] = array(
    '#type' => 'textfield',
    '#title' => t('Hash Key'),
    '#default_value' => $default['hashkey'],
    '#size' => 20,
  );
  $form['hashiv'] = array(
    '#type' => 'textfield',
    '#title' => t('Hash IV'),
    '#default_value' => $default['hashiv'],
    '#size' => 20,
  );
  $form['merchantid_test'] = array(
    '#type' => 'textfield',
    '#title' => t('Merchant ID').' ('.t('Test').')',
    '#default_value' => $default['merchantid_test'],
    '#size' => 20,
  );
  $form['hashkey_test'] = array(
    '#type' => 'textfield',
    '#title' => t('Hash Key').' ('.t('Test').')',
    '#default_value' => $default['hashkey_test'],
    '#size' => 20,
  );
  $form['hashiv_test'] = array(
    '#type' => 'textfield',
    '#title' => t('Hash IV').' ('.t('Test').')',
    '#default_value' => $default['hashiv_test'],
    '#size' => 20,
  );
  $form['delete'] = array(
    '#type' => 'checkbox',
    '#title' => t('Delete'),
    '#default_value' => 0,
  );
  return $form;
}
