<?php

namespace Drupal\esunbank\Form;

use Drupal\Core\Form\ConfigFormBase;
use Drupal\Core\Form\FormStateInterface;

/**
 * Defines a form that configures forms module settings.
 */
class EsunbankConfigurationForm extends ConfigFormBase {

  /**
   * {@inheritdoc}
   */
  public function getFormId() {
    return 'esunbank_admin_settings';
  }

  /**
   * {@inheritdoc}
   */
  protected function getEditableConfigNames() {
    return [
      'esunbank.admin_settings',
    ];
  }

  /**
   * {@inheritdoc}
   */
  public function buildForm(array $form, FormStateInterface $form_state) {
    $config = $this->config('esunbank.admin_settings');
    $form['contact_info'] = [
      '#type' => 'textarea',
      '#title' => $this->t('Organization Contact Info'),
      '#default_value' => $config->get('contact_info'),
    ];
    $form['receipt_info'] = [
      '#type' => 'textarea',
      '#title' => $this->t('Organization Receipt Info'),
      '#default_value' => $config->get('receipt_info'),
    ];
    $form['bank_account_name'] = [
      '#type' => 'textfield',
      '#title' => $this->t('Bank Account Name'),
      '#default_value' => $config->get('bank_account_name'),
    ];
    $form['postoffice_account_number'] = [
      '#type' => 'textarea',
      '#title' => $this->t('Postoffice Account Number'),
      '#default_value' => $config->get('postoffice_account_number'),
    ];
    return parent::buildForm($form, $form_state);
  }

  /**
   * {@inheritdoc}
   */
  public function submitForm(array &$form, FormStateInterface $form_state) {
    $this->config('esunbank.admin_settings')
      ->set('contact_info', $form_state->getValue('contact_info'))
      ->set('receipt_info', $form_state->getValue('receipt_info'))
      ->set('bank_account_name', $form_state->getValue('bank_account_name'))
      ->set('postoffice_account_number', $form_state->getValue('postoffice_account_number'))
      ->save();
    parent::submitForm($form, $form_state);
  }

}
