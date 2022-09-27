<?php

namespace Drupal\esunbank\Controller;

use Drupal\Core\Controller\ControllerBase;
use Symfony\Component\HttpFoundation\Response;
use Symfony\Component\HttpFoundation\JsonResponse;

class EsunbankController extends ControllerBase {
  public static function esunbankAPI() {
    $unsafe = \Drupal::request()->query->get('params');
    $params = explode('|', \Drupal\Component\Utility\Xss::filter($unsafe));

    $vars = array(
      'code' => $params[0],
      'number' => $params[1],
      'length' => $params[2],
      'price' => $params[3],
      'timestamp' => $params[4],
      'store' => $params[5],
      'type' => $params[6],
      'user' => $params[7],
      'item' => $params[8],
    );
    $html = esunbank_api($vars);
    return new Response($html);
  }

  public static function barcode39($serial) {
    if (preg_match_all('/[^0-9a-z *$+%.\/-]/ui', $serial, $matches)) {
      $json = [
        'error' => 'Invalid character(s) for code 39: '.implode('', $matches[0]),
      ];
      return new JsonResponse($json, 400);
    }
    $barcode_image = esunbank_api_barcode_image($serial);

    $response = new Response();
    $response->headers->set('Cache-Control', 'private');
    $response->headers->set('Content-Type', 'image/gif');

    // Send headers before outputting anything
    $response->sendHeaders();
    $response->setContent($barcode_image);

    return $response;

  }
}