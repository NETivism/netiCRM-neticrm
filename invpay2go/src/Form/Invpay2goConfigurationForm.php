<?php

namespace Drupal\invpay2go\Form;

use Drupal\Core\Form\ConfigFormBase;
use Drupal\Core\Form\FormStateInterface;

/**
 * Defines a form that configures forms module settings.
 */
class Invpay2goConfigurationForm extends ConfigFormBase {

  /**
   * {@inheritdoc}
   */
  public function getFormId() {
    return 'invpay2go_admin_settings';
  }

  /**
   * {@inheritdoc}
   */
  protected function getEditableConfigNames() {
    return [
      'invpay2go.admin_settings',
    ];
  }

  /**
   * {@inheritdoc}
   */
  public function buildForm(array $form, FormStateInterface $form_state) {
    $form = array();
    $db = \Drupal::database()->select('invpay2go', 'inv')
      ->fields('inv')
      ->orderBy('pid', 'ASC')
      ->execute();
    $form['#tree'] = TRUE;
    while($payment = $db->fetchAssoc()) {
      $pid = $payment['pid'];
      $container = self::singleSettingForm($pid, $payment);
      $form['container'][$pid] = $container;
    }
    $pid++;
    $form['container'][$pid] = self::singleSettingForm($pid, array());
    $form['container'][$pid]['#collapsible'] = TRUE;
    $form['container'][$pid]['#collapsed'] = TRUE;
    return parent::buildForm($form, $form_state);
  }

  function singleSettingForm($pid, $default){
    $title = !empty($default) ? t('Pay2go Settings'). ': '.$pid : t('New Pay2go Settings');
    $form = array();
    $form = array(
      '#type' => 'details',
      '#title' => $title,
      '#open' => !empty($default) ? TRUE : FALSE,
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

  /**
   * {@inheritdoc}
   */
  public function submitForm(array &$form, FormStateInterface $form_state) {
    $settings = \Drupal::database()->select('invpay2go', 'inv')
      ->fields('inv')
      ->execute()
      ->fetchAll();
    $exists = array();
    foreach($settings as $e){
      $exists[$e->pid] = (array) $e;
    }
    $container = $form_state->getValue('container');
    foreach ($container as $pid => $invoice_setting) {
      if (empty($invoice_setting)) {
        continue;
      }
      if (isset($exists[$pid]) && !empty($exists[$pid])) {
        if (!empty($invoice_setting['delete'])) {
          \Drupal::database()->delete('invpay2go')
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
          \Drupal::database()->update('invpay2go')
            ->fields($fields)
            ->condition('pid', $pid, '=')
            ->execute();
        }
      }
      else{
        if(!empty($invoice_setting['hashkey']) || !empty($invoice_setting['hashiv']) || !empty($invoice_setting['hashkey_test']) || !empty($invoice_setting['hashiv_test'])) {
          \Drupal::database()->insert('invpay2go')
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
  }
}
