<?php
class spgateway_spgateway_api {

  public $_urlDomain;

  // Used for request parameter
  public $_isTest;
  public $_paymentProcessorId;
  public $_paymentProcessor;
  public $_apiType;
  public $_apiMethod = 'POST'; // In This API , Always Use POST.

  public $_contribution_id; // this request relative contribution.
  public $_logId;

  public static $_apiTypes = array(
    'alter-status' => '/MPG/period/AlterStatus',
    'alter-amt' => '/MPG/period/AlterAmt',
  );
  public static $_alterStatus = array(
    'suspend',    // Paused
    'terminate',  // Stop
    'restart',    // Only used in paused recur.
  );

  // Used for request result
  public $_apiURL;
  public $_request;
  public $_response;
  public $_success;


  /**
   * $apiParams must has these fields: 
   *   apiType
   *   paymentProcessorId or paymentProcessor
   *   isTest
   */
  function __construct($apiParams) {
    extract($apiParams);
    if (!empty($paymentProcessor)) {
      $this->_paymentProcessor = $paymentProcessor;
    }
    else if (!empty($paymentProcessorId)) {
      $mode = $isTest ? '' : 'test';
      $this->_paymentProcessor = CRM_Core_BAO_PaymentProcessor::getPayment($paymentProcessorId, $mode);
    }
    else {
      CRM_Core_Error::fatal('Missing payment processor or payment processor ID');
    }
    if ($isTest) {
      $this->_urlDomain = SPGATEWAY_TEST_DOMAIN;
    }
    else {
      $this->_urlDomain = SPGATEWAY_REAL_DOMAIN;
    }
    $this->_isTest = $isTest;
    $this->_paymentProcessorId = $paymentProcessorId;
    $this->_apiType = $apiType;
  }

  /*
   *
   * ########  ########  #######  ##     ## ########  ######  ######## 
   * ##     ## ##       ##     ## ##     ## ##       ##    ##    ##    
   * ##     ## ##       ##     ## ##     ## ##       ##          ##    
   * ########  ######   ##     ## ##     ## ######    ######     ##    
   * ##   ##   ##       ##  ## ## ##     ## ##             ##    ##    
   * ##    ##  ##       ##    ##  ##     ## ##       ##    ##    ##    
   * ##     ## ########  ##### ##  #######  ########  ######     ##    
   *
   * Required field:
   * * MerOrderNo
   * * PeriodNo
   * 
   */
  public function request($params) {
    $allowedFields = self::fields($this->_apiType);
    $post = array();
    foreach ($params as $name => $value) {
      if (!in_array($name, $allowedFields)) {
        continue;
      }
      else {
        $post[$name] = $value;
      }
    }
    if (empty($post['RespondType'])) {
      $post['RespondType'] = SPGATEWAY_RESPONSE_TYPE;
    }
    if (empty($post['Version'])) {
      $post['Version'] = SPGATEWAY_RECUR_VERSION;
    }
    if (empty($post['TimeStamp'])) {
      $post['TimeStamp'] = time();
    }

    $requiredFields = self::fields($this->_apiType, TRUE);
    foreach ($requiredFields as $required) {
      if(empty($post[$required])){
        $missingRequired[] = $required;
      }
    }
    if(!empty($missingRequired)) {
      CRM_Core_Error::fatal('Required parameters missing: '.implode(',', $missingRequired));
    }

    if (!empty($post['PeriodType']) xor !empty($post['PeriodPoint'])) {
      CRM_Core_Error::fatal('PeriodType and PeriodPoint must exist at same time.');
    }
    if (!empty($post['PeriodType'])) {
      if ($post['PeriodType'] == 'Y' && !preg_match('/\d{4}/', $post['PeriodPoint'])) {
        CRM_Core_Error::fatal('PeriodPoint format should be MMDD when PeriodType is "Y".');
      }
      if ($post['PeriodType'] == 'M' && !preg_match('/\d{2}/', $post['PeriodPoint'])) {
        CRM_Core_Error::fatal('PeriodPoint format should be DD when PeriodType is "M".');
      }
    }

    if (!empty($post['PeriodTimes']) xor !empty($post['Extday'])) {
      CRM_Core_Error::fatal('PeriodTimes and Extday must exist at same time.');
    }

    $this->_request = $post;
    $this->_apiURL = $this->_urlDomain.self::$_apiTypes[$this->_apiType];
    $result = $this->_curl();
    if ($result['status'] && !empty($this->_response)) {

      // Format of amount
      $response =& $this->_response;
      if(!empty($response->amount) && $response->currency != 'TWD') {
        $response->amount = (float)$response->amount / 100;
      }

      return $this->_response;
    }
    else {
      return FALSE;
    }
  }

  /*
   * 
   *  ######  ##     ## ########  ##       
   * ##    ## ##     ## ##     ## ##       
   * ##       ##     ## ##     ## ##       
   * ##       ##     ## ########  ##       
   * ##       ##     ## ##   ##   ##       
   * ##    ## ##     ## ##    ##  ##       
   *  ######   #######  ##     ## ######## 
   * 
   */
  private function _curl() {
    $this->_success = FALSE;
    if (!empty(getenv('CIVICRM_TEST_DSN'))) {
      return  array(
        'success' => FALSE,
        'status' => NULL,
        'curlError' => NULL,
      );
    }
    $ch = curl_init($this->_apiURL);
    $opt = array();
    $opt[CURLOPT_RETURNTRANSFER] = TRUE;
    if($this->_apiMethod == 'POST'){
      $requestString = http_build_query($this->_request, '', '&');
      $postDataString = _civicrm_spgateway_recur_encrypt($requestString, $this->_paymentProcessor);
      $postFields = array(
        'MerchantID_' => $this->_paymentProcessor['user_name'],
        'PostData_' => $postDataString,
      );
      $opt[CURLOPT_POST] = TRUE;
      $opt[CURLOPT_POSTFIELDS] = $postFields;
    }
    curl_setopt_array($ch, $opt);

    // $recordData = array(
    //   'contribution_id' => $this->_contribution_id,
    //   'url' => $this->_apiTypes[$this->_apiType],
    //   'date' => date('Y-m-d H:i:s'),
    //   'post_data' => $opt[CURLOPT_POSTFIELDS],
    // );
    // $lodId = $this->writeRecord(NULL, $recordData);

    $result = curl_exec($ch);

    // $recordData = array(
    //   'return_data' => $result,
    // );
    // $this->writeRecord($lodId, $recordData);

    $status = curl_getinfo($ch, CURLINFO_HTTP_CODE);
    $errno = curl_errno($ch);
    if (!empty($errno)) {
        $errno = curl_errno($ch);
        $err = curl_error($ch);
        CRM_Core_Error::debug_log_message("CURL: $err :: $errno");
    }

    if ($result === FALSE) {
      $errno = curl_errno($ch);
      $err = curl_error($ch);
      $curlError = array($errno => $err);
    }
    else{
      $curlError = array();
    }
    curl_close($ch);
    if (!empty($result)) {
      $response = json_decode($result);
      $this->_response = json_decode(_civicrm_spgateway_recur_decrypt($response->period, $this->_paymentProcessor));
      $this->_success = isset($response->status) && $response->status == '0' ? TRUE : FALSE;
    }
    else {
      $this->_response = NULL;
    }
    $return = array(
      'success' => $this->_success,
      'status' => $status,
      'curlError' => $curlError,
    );
    return $return;
  }

  /*
   *
   * ########  ########  ######   #######  ########  ########  
   * ##     ## ##       ##    ## ##     ## ##     ## ##     ## 
   * ##     ## ##       ##       ##     ## ##     ## ##     ## 
   * ########  ######   ##       ##     ## ########  ##     ## 
   * ##   ##   ##       ##       ##     ## ##   ##   ##     ## 
   * ##    ##  ##       ##    ## ##     ## ##    ##  ##     ## 
   * ##     ## ########  ######   #######  ##     ## ########  
   *
   */
  public function writeRecord($logId, $data) {
    $record = new CRM_Core_DAO_Log();
    if(!empty($logId)) {
      $record->id = $logId;
      $record->find(TRUE);
    }

    $record->entity_table = 'spgateway_api_'.$this->_apiType;
    $record->entity_id = $this->_contribution_id;

    $record->data = serialize($data);

    $record->save();
    return $record->id;
  }

  public static function fields($apiType, $is_required = FALSE) {
    $fields = array();
    switch($apiType){
      case 'alter-status':
        $fields = explode(',', 'RespondType*,Version*,MerOrderNo*,PeriodNo*,AlterType*,TimeStamp*');
        break;
      case 'alter-amt':
        $fields = explode(',', 'RespondType*,Version*,TimeStamp*,MerOrderNo*,PeriodNo*,AlterAmt,PeriodType,PeriodPoint,PeriodTimes,Extday');
        break;
    }
    foreach ($fields as $key => &$value) {
      if(!strstr($value, '*') && $is_required) {
        unset($fields[$key]);
      }
      else{
        $value = str_replace('*', '', $value);
      }
    }
    return $fields;
  }

}