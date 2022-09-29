<?php

namespace Drupal\civicrm_esunbank\Controller;

use Drupal\Core\Controller\ControllerBase;
use Symfony\Component\HttpFoundation\Response;

class CivicrmEsunbankController extends ControllerBase {
  public static function ipn() {
    \Drupal::service('civicrm')->initialize();
    \Drupal::moduleHandler()->loadInclude('civicrm_esunbank', 'inc', 'civicrm_esunbank.ipn');
    $return = civicrm_esunbank_ipn(NULL, TRUE);

    $response = new Response();
    $response->headers->set('Cache-Control', 'private');
    $response->headers->set('Content-Type', 'text/plain');
    if (!empty($return)) {
      $response->setContent('OK');
    }
    else {
      $response->setContent('Error');
    }
    return $response;
  }
}