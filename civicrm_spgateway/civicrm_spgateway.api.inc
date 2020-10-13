<?php
class spgateway_spgateway_api {

  protected $_urlDomain;

  // Used for request parameter
  protected $_apiType;

  protected $_contribution_id; // this request relative contribution.
  protected $_apiMethod; // In This API , Always Use POST.

  protected static $_apiTypes = array(
    'alter-status' => '/MPG/period/AlterStatus',
    'alter-amt' => '/MPG/period/AlterAmt',
  );
  protected static $_alterStatus = array(
    'suspend',    // Paused
    'terminate',  // Stop
    'restart',    // Only used in paused recur.
  );

  // Used for request result
  protected $_response;
  protected $_success;


  /**
   * $apiParams must has these fields: 
   *   apiType
   *   partnerKey
   *   isTest
   */
  function __construct($apiParams) {
    extract($apiParams);
  }

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

    $requiredFields = self::fields($this->_apiType, TRUE);
    foreach ($requiredFields as $required) {
      if(empty($post[$required])){
        $missingRequired[] = $required;
      }
    }
    if(!empty($missingRequired)) {
      CRM_Core_Error::fatal('Required parameters missing: '.implode(',', $missingRequired));
    }

    $this->_request = $post;
    $result = $this->_curl();
    if ($result['status'] && !empty($this->_response)) {
      if (in_array($this->_apiType, $this->_apiNeedSaveData)) {
        // Record tappay data
        self::saveTapPayData($this->_contribution_id, $this->_response, $this->_apiType);
      }

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

    $opt[CURLOPT_HTTPHEADER] = array(
      'Content-Type: application/json',
      'x-api-key: ' . $this->_partnerKey,
    );
    $opt[CURLOPT_RETURNTRANSFER] = TRUE;
    if($this->_apiMethod == 'POST'){
      $opt[CURLOPT_POST] = TRUE;
      $opt[CURLOPT_POSTFIELDS] = json_encode($this->_request, JSON_UNESCAPED_UNICODE | JSON_PRETTY_PRINT);
    }
    curl_setopt_array($ch, $opt);

    $recordData = array(
      'contribution_id' => $this->_contribution_id,
      'url' => $this->_apiTypes[$this->_apiType],
      'date' => date('Y-m-d H:i:s'),
      'post_data' => $opt[CURLOPT_POSTFIELDS],
    );
    $lodId = self::writeRecord(NULL, $recordData);

    $result = curl_exec($ch);

    $recordData = array(
      'return_data' => $result,
    );
    self::writeRecord($lodId, $recordData);

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
      $this->_response = $response;
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

  protected static function fields($apiType, $is_required = FALSE) {
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