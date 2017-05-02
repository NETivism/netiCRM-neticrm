<?php

/**
 * Form builder; Configure invpay2go secret settings
 *
 * @ingroup forms
 * @see system_settings_form()
 */
function invpay2go_admin_settings() {
  $form = array();
  $query = db_query("SELECT * FROM {invpay2go} ORDER BY pid ASC");
  $form['#tree'] = TRUE;
  while($payment = db_fetch_array($query)) {
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
  $query = db_query("SELECT * FROM {invpay2go} ORDER BY pid ASC");
  $settings = array();
  while ($inv = db_fetch_object($query)) {
    $settings[$inv->pid] = $inv;
  }
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
        db_query("DELETE FROM {invpay2go} WHERE pid = %d", $pid);
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
          'pid' => $pid,
        );
        drupal_write_record('invpay2go', $fields, 'pid');
      }
    }
    else{
      if(!empty($invoice_setting['hashkey']) || !empty($invoice_setting['hashiv']) || !empty($invoice_setting['hashkey_test']) || !empty($invoice_setting['hashiv_test'])) {
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
        drupal_write_record('invpay2go', $fields);
      }
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
