<?php

class Pay2goInvoice {

  public const INVPAY2GO_TEST = 'https://cinv.ezpay.com.tw';
  public const INVPAY2GO_PROD = 'https://inv.ezpay.com.tw';
  public const RESPONSE_TYPE = 'JSON';

  public $isTest = 0;
  public $status = 0;
  public $response;
  public $requestParams;
  public $orderNumber;
  public $checkResult;
  public $pid;
  public $recordField;

  protected $merchantId;
  protected $hashKey;
  protected $hashIv;

  public $apiType;

  // api type => version
  protected $apiTypes = [
    'invoice_issue' => '1.4',
    'invoice_touch_issue' => '1.0',
    'invoice_search' => '1.1',
    // not supportted api types
    'invoice_invalid' => 0,
    'allowance_issue' => 0,
    'allowance_touch_issue' => 0,
  ];

  protected $apiVersions = [

  ];

  /**
   * Initialize a connection
   *
   * @param $apiParams
       - string $merchantId
   *     merchantID provided by pay2go
   *   - string $key
   *     Hash key provided by pay2go
   *   - string $iv
   *     hash iv provided by pay2go
   *   - string $apiType
   *     api type which needs call
   * @param bool $isTest
   *   A boolean indicate if this api call test or not.
   */
  public function __construct($apiParams, $isTest) {
    extract($apiParams);
    if (empty($merchantId) || empty($key) || empty($iv) || is_null($isTest)) {
      throw new Exception('Required parameters missing: merchantId, key, iv, isTest');
    }
    if ($isTest) {
      $obj = \Drupal::database()->query("SELECT pid, hashkey_test as hashkey, hashiv_test as hashiv FROM {invpay2go} WHERE merchantid_test = :mid", [':mid' => $merchantId])->fetchObject();
    }
    else {
      $obj = \Drupal::database()->query("SELECT pid, hashkey, hashiv FROM {invpay2go} WHERE merchantid = :mid", [':mid' => $merchantId])->fetchObject();
    }
    if (empty($obj->pid)) {
      throw new Exception('Specify merchant id  not found in db.');
    }
    if ($obj->hashkey != $key || $obj->hashiv != $iv) {
      throw new Exception('Hash key or iv not correct between api and db');
    }
    if (empty($this->apiTypes[$apiType])) {
      throw new Exception('API type not supported currently or given wrong type');
    }
    $this->pid = $obj->pid;
    $this->apiType = $apiType;
    $this->recordField = $this->recordType();
    $this->merchantId = $merchantId;
    $this->hashKey = $key;
    $this->hashIv = $iv;
    $this->isTest = $isTest;
  }

  /**
   * Request a http query to pay2go
   *
   * @param array $params
   *   key-value array of allowed fields prepare to query
   * @param boolean $verifyResponse
   *   specify FALSE to skip checkcode check from response
   *   default is TRUE to check response comes from same origin
   */
  public function request($params, $verifyResponse = TRUE) {
    $allowedFields = self::fields($this->apiType);
    $postData = [];
    foreach ($params as $key => $value) {
      if (!in_array($key, $allowedFields)) {
        continue;
      }
      else {
        if ($key == 'ItemName') {
          $value = \Drupal\Component\Utility\Unicode::truncate($value, 28);
        }
        $postData[$key] = $value;
      }
    }
    if (empty($postData['MerchantOrderNo'])) {
      $errorMessage = "Missing required parameter: MerchantOrderNo";
      throw new Exception($errorMessage);
    }
    else {
      $this->orderNumber = $postData['MerchantOrderNo'];
    }

    // force version and RespondType
    $postData['RespondType'] = self::RESPONSE_TYPE;
    $postData['Version'] = empty($postData['Version']) ? $this->apiTypes[$this->apiType] : $postData['Version'];
    $postData['TimeStamp'] = (string) time();

    $postStr = http_build_query($postData, '', '&');
    $encryptedPostData = self::encrypt($postStr, $this->hashKey, $this->hashIv);

    $transactionData = [
      'MerchantID_' => $this->merchantId,
      'PostData_' => $encryptedPostData,
    ];
    $transactionDataStr = http_build_query($transactionData, '', '&');
    $resp = self::curlPost($this->apiUrl($this->apiType), $transactionDataStr);
    $data = self::parseResponse($this->apiType, $resp);

    $record = [
      'order_number' => $this->orderNumber,
      'pid' => $this->pid,
      'is_test' => $this->isTest,
    ];
    if (isset($data->Result->InvoiceNumber)) {
      $record['invoice_number'] = $data->Result->InvoiceNumber;
    }
    $record[$this->recordField] = $resp['response'];
    self::writeRecord($this->orderNumber, $record);

    // verify data comes from
    if (isset($data->Result->CheckCode) && $verifyResponse) {
      $checkResult = self::checkCode($data, $this->hashKey, $this->hashIv);
      $this->checkResult = $checkResult;
    }
    $this->status = 0;
    if ($verifyResponse && $this->checkResult && $data->Status == 'SUCCESS') {
      $this->status = 1;
    }
    elseif ($data->Status == 'SUCCESS') {
      $this->status = 1;
    }

    $this->response[$this->recordField] = $data;
    return $data;
  }

  /**
   * Free current response object
   */
  public function free() {
    unset($this->response);
  }

  /**
   * Record type
   */
  private function recordType() {
    $recordField = '';
    switch ($this->apiType) {
      case 'invoice_issue':
      case 'invoice_touch_issue':
        $recordField = 'issue_response';
        break;
      case 'invoice_search':
        $recordField = 'search_response';
        break;
      case 'invoice_invalid':
        $recordField = 'invalid_response';
        break;
      case 'allowance_issue':
      case 'allowance_touch_issue':
        $recordField = 'allowance_response';
        break;
    }
    $this->recordField = $recordField;
    return $recordField;
  }

  /**
   * Base api url for api
   */
  public function apiUrl() {
    if ($this->isTest) {
      return self::INVPAY2GO_TEST . '/Api/' . $this->apiType;
    }
    else {
      return self::INVPAY2GO_PROD. '/Api/' . $this->apiType;
    }
  }

  /**
   * Decode result
   */
  public static function parseResponse($type, $resp) {
    $errno = key($resp['curlError']);
    $err = reset($resp['curlError']);
    if (($resp['httpStatus'] && $resp['httpStatus'] != 200) || !empty($err)) {
      $errorMessage = "Response Error:HTTPStatusCode-{$resp['httpStatus']},CURL-{$errno}-{$err}";
      throw new Exception($errorMessage);
    }
    if (!empty($resp['response'])) {
      $data = json_decode($resp['response']);
      if (!empty($data->Result)) {
        $result = json_decode($data->Result);
        unset($data->Result);
        $data->Result = $result;
      }
      return $data;
    }
    else {
      throw new Exception('Empty response');
    }
    return FALSE;
  }

  /**
   * Helper function for curl post data
   */
  public static function curlPost($url, $postData) {
    $curlOptions = [
      CURLOPT_URL => $url,
      CURLOPT_HEADER => FALSE,
      CURLOPT_RETURNTRANSFER => TRUE,
      CURLOPT_USERAGENT => 'pay2go client',
      CURLOPT_FOLLOWLOCATION => TRUE,
      CURLOPT_SSL_VERIFYPEER => FALSE,
      CURLOPT_SSL_VERIFYHOST => FALSE,
      CURLOPT_POST => '1',
      CURLOPT_POSTFIELDS => $postData,
    ];
    $ch = curl_init();
    curl_setopt_array($ch, $curlOptions);
    $result = curl_exec($ch);
    $httpStatus = curl_getinfo($ch, CURLINFO_HTTP_CODE);
    if ($result === FALSE) {
      $errno = curl_errno($ch);
      $err = curl_errno($ch);
      $curlError = [$errno => $err];
    }
    else {
      $curlError = [];
    }
    curl_close($ch);
    return [
      'httpStatus' => $httpStatus,
      'curlError' => $curlError,
      'response' => $result
    ];
  }

  /**
   * Helper function for get record
   */
  public static function getRecord($order_number = NULL, $invoice_number = NULL) {
    if (!empty($order_number)) {
      $record = \Drupal::database()->query("SELECT * FROM {invpay2go_invoice} WHERE order_number = :or", [':or' => $order_number])->fetchObject();
    }
    elseif (!empty($invoice_number)) {
      $record = \Drupal::database()->query("SELECT * FROM {invpay2go_invoice} WHERE invoice_number = :in", [':in' => $invoice_number])->fetchObject();
    }
    if (!empty($record->order_number)) {
      $return = [];
      foreach ($record as $key => $r) {
        if (strstr($key, 'response')) {
          $json = json_decode($r);
          if ($json) {
            foreach ($json as $k => $v) {
              if ($k == 'Result' && !empty($v)) {
                $json->Result = json_decode($v);
              }
            }
            $return[$key] = $json;
          }
        }
        else {
          $return[$key] = $r;
        }
      }
      return $return;
    }
  }

  /**
   * Helper function to get full array
   */
  public static function getInfo($object) {
    $info = [];
    if (!empty($object['issue_response']->Result)) {
      $info = (array) $object['issue_response']->Result;
    }
    if (!empty($object['search_response']->Result)) {
      $result = (array) $object['search_response']->Result;
      $result['ItemDetail'] = json_decode($result['ItemDetail']);
      $info = array_merge($info, $result);
    }
    $merchant = invpay2go_merchant_load($object['pid'], ['title', 'serial']);
    $info = array_merge($info, $merchant);
    return $info;
  }

  /**
   * Helper function for write record
   * @param string $orderNumber
   *   orderNumber identifier
   * @param array $record
   *   order_number is required when update
   *   pid is required if there is a invoice system id
   *   invoice_number is string
   *   issue_response is flatten json object in string
   *   search_response is flatten json object in string
   *   invalid_response is flatten json object in string
   *   allowance_response is flatten json object in string
   */
  public static function writeRecord($orderNumber, $record) {
    if (!empty($orderNumber)) {
      $exists = \Drupal::database()->query("SELECT order_number FROM {invpay2go_invoice} WHERE order_number = :order_number", [':order_number' => $orderNumber])->fetchObject();
    }
    \Drupal::database()->merge('invpay2go_invoice')
      ->keys(['order_number' => $exists->order_number])
      ->fields($record)
      ->execute();
  }

  /**
   * Helper function for printing tax receipt
   * @param mix $object
   *   object must have InvoiceNumber to print
   *   If it's string, get receipt info from db
   *   If it's object, check invoice number
   * @return string
   *   Return printing html with inline css style prepared for printing
   */
  public static function printReceipt($object) {
    $print = '';
    if (is_string($object)) {
      $object = self::getRecord($object);
    }
    $info = self::getInfo($object);
    if (!empty($info['QRcodeL'])) {
      $info['QRcodeLimg'] = invpay2go_api_qrcode($info['QRcodeL']);
    }
    if (!empty($info['QRcodeR'])) {
      $info['QRcodeRimg'] = invpay2go_api_qrcode($info['QRcodeR']);
    }
    if (!empty($info['CreateTime']) && !empty($info['InvoiceNumber']) && !empty($info['RandomNum'])) {
      $create = strtotime($info['CreateTime']);
      $cyear = (int) date('Y', $create);
      $cyear -= 1911;
      $cmonth = date('m', $create);
      $info['BarcodeCode'] = $cyear.$cmonth.$info['InvoiceNumber'].$info['RandomNum'];
      $info['Barcode'] = invpay2go_api_barcode($info['BarcodeCode']);
    }
    if (!empty($info['InvoiceNumber'])) {
      $element['invpay2go'] = [
        '#theme' => 'invpay2go',
        '#receipt' => $info,
      ];
      $print = \Drupal::service('renderer')->render($element);
    }
    return $print;
  }

  /**
   * Helper function for add padding
   */
  public static function padding($string, $blocksize = 32) {
    $len = strlen($string);
    $pad = $blocksize - ($len % $blocksize);
    $string .= str_repeat(chr($pad), $pad);
    return $string;
  }

  /**
   * Helper function for encrypt
   */
  public static function encrypt($string, $key, $iv) {
    $data = self::padding($string);
    $openssl = extension_loaded('openssl') ? TRUE : FALSE;
    $mcrypt = extension_loaded('mcrypt') ? TRUE : FALSE;
    if (empty($openssl) && empty($mcrypt)) {
      return FALSE;
    }

    if ($openssl) {
      $keyLen = strlen($key);
      switch ($keyLen) {
        case 16:
          $encoding = 'AES-128-CBC';
          break;
        case 24:
          $encoding = 'AES-192-CBC';
          break;
        case 32:
        default:
          $encoding = 'AES-256-CBC';
          break;
      }
      $encrypted = openssl_encrypt($data, $encoding, $key, OPENSSL_RAW_DATA | OPENSSL_ZERO_PADDING, $iv);
    }
    elseif ($mcrypt) {
      $encrypted = mcrypt_encrypt(MCRYPT_RIJNDAEL_128, $key, $data, MCRYPT_MODE_CBC, $iv);
    }
    else {
      return '';
    }
    return trim(bin2hex($encrypted));
  }

  /**
   * Helper functioh for check code
   */
  public static function checkCode($result, $key, $iv) {
    if (!is_array($result)) {
      $result = (array) $result;
    }
    $code = $result['CheckCode'];
    unset($result['CheckCode']);
    ksort($result);
    $check = http_build_query($result, '', '&');
    $check = 'HashIV='.$iv.'&'.$check.'&HashKey='.$key;
    $check = strtoupper(hash("sha256", $check));
    if ($check === $code) {
      return TRUE;
    }
    else {
      return FALSE;
    }
  }

  /**
   * API query fields
   */
  public static function fields($type, $isResponse = FALSE) {
    $fields = [];
    if ($isResponse) {
      switch ($type) {
        case 'invoice_issue':
          $fields = explode(',', 'MerchantID,InvoiceTransNo,MerchantOrderNo,TotalAmt,InvoiceNumber,RandomNum,CreateTime,CheckCode,BarCode,QRcodeL,QRcodeR');
          break;
        case 'invoice_touch_issue':
          $fields = explode(',', 'MerchantID,InvoiceTransNo,MerchantOrderNo,TotalAmt,InvoiceNumber,RandomNum,CreateTime,CheckCode');
          break;
        case 'invoice_search':
          $fields = explode(',', 'MerchantID,InvoiceTransNo,MerchantOrderNo,InvoiceNumber,RandomNum,BuyerName,BuyerUBN,BuyerAddress,BuyerPhone,BuyerEmail,InvoiceType,Category,TaxType,TaxRate,Amt,AmtSales,AmtZero,AmtFree,TaxAmt,TotalAmt,CarrierType,CarrierNum,LoveCode,PrintFlag,CreateTime,ItemDetail,InvoiceStatus,UploadStatus,CheckCode');
          break;
      }
    }
    else {
      switch ($type) {
        case 'invoice_issue':
          $fields = explode(',', 'RespondType,Version,TimeStamp,TransNum,MerchantOrderNo,Status,CreateStatusTime,Category,BuyerName,BuyerUBN,BuyerAddress,BuyerEmail,CarrierType,CarrierNum,LoveCode,PrintFlag,TaxType,TaxRate,CustomsClearance,Amt,AmtSales,AmtZero,AmtFree,TaxAmt,TotalAmt,ItemName,ItemCount,ItemUnit,ItemPrice,ItemAmt,ItemTaxType,Comment');
          break;
        case 'invoice_touch_issue':
          $fields = explode(',', 'RespondType,Version,TimeStamp,TransNum,InvoiceTransNo,MerchantOrderNo,TotalAmt');
          break;
        case 'invoice_search':
          $fields = explode(',', 'RespondType,Version,TimeStamp,SearchType,MerchantOrderNo,TotalAmt,InvoiceNumber,RandomNum,DisplayFlag');
          break;
      }
    }
    return $fields;
  }
}
