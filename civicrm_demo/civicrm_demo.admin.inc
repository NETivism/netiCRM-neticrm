<?php
module_load_include('inc', 'civicrm_demo', 'civicrm_demo.utils');

function civicrm_demo_settings(){
  $form['operation'] = array(
    '#type' => 'radios',
    '#required' => true,
    '#title' => t('Data'),
    '#options' => array(
      'contact' => t('Contact'),
      'contribution' => t('Contribution'),
      'event' => t('Event and Participant'),
      'member' => t('Member'),
    ),
  );
  $form['limit'] = array(
    '#type' => 'textfield',
    '#title' => t('Number'),
    '#default_value' => 10,
  );
  $form['start_year'] = array(
    '#type' => 'textfield',
    '#title' => t('Start year'),
    '#default_value' => date('Y', (time() - 86400*5*365) ),
    '#states' => array(
      'visible' => array(   // action to take.
        ':input[name="operation"]' => array('value' => 'contribution'),
      ),
    ),
  );
  $form['batch'] = array(
    '#type' => 'checkbox',
    '#title' => t('Batch'),
    '#default_value' => 0,
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Generate'),
  );
  return $form;
}

function civicrm_demo_settings_submit($form, &$form_state){
  set_time_limit(1200);
  $op = $form_state['values']['operation'];
  $function = 'civicrm_demo_process_'.$op;
  $form_state['values']['function'] = $function;
  if(function_exists($function)){
    if($form_state['values']['batch']){
      civicrm_demo_batch($form_state['values']);
    }
    else{
      $function($form_state['values']);
      drupal_set_message(t("Successful generate data of !type", array('!type' => $op )));
    }
  }
}

function civicrm_demo_batch($vars) {
  $batch = array(
    'operations' => array(
      array('civicrm_demo_batch_process', array($vars)),
    ),
    'finished' => 'civicrm_demo_batch_finished',
    'title' => t('Processing').' '.$vars['operation'],
  );
  batch_set($batch);
}

function civicrm_demo_batch_process($vars, &$context){
  if (!isset($context['sandbox']['progress'])) {
    $context['sandbox']['progress'] = 0;
    $context['sandbox']['max'] = $vars['limit'];
    $context['message'] = 'Starting ...';
  }
  $num_per_batch = 50;
  $arg = array('limit' => $num_per_batch);
  $func = $vars['function'];
  $func($arg);
  $context['sandbox']['progress'] += $num_per_batch;
  $context['message'] = $context['sandbox']['progress'] .' of '.$context['sandbox']['max'];

  if ($context['sandbox']['progress'] < $context['sandbox']['max']) {
    $context['finished'] = $context['sandbox']['progress'] / $context['sandbox']['max'];
  }
  else{
    $context['finished'] = 1;
  }
}

function civicrm_demo_batch_finished($success, $results, $operations){
  if ($success) {
    drupal_set_message('Processed complete');
  }
  else {
    // An error occurred.
    // $operations contains the operations that remained unprocessed.
    $error_operation = reset($operations);
    $message = t('An error occurred while processing %error_operation with arguments: @arguments', array('%error_operation' => $error_operation[0], '@arguments' => print_r($error_operation[1], TRUE)));
    drupal_set_message($message, 'error');
  }
}

