<?php

function _neticrm_update_update_7247(){
  civicrm_initialize();
  $sql = "SELECT @@GLOBAL.innodb_file_format;";
  $value = CRM_Core_DAO::singleValueQuery($sql);
  if($value == 'Antelope'){
    CRM_Core_Error::debug_log_message("Database variable 'innodb_file_format' is not 'Barracuda', Stop update.", true);
    return False;
  }

  $converter = new CRMCharsetConverter($charset, $collation);
  $dao = CRM_Core_DAO::executeQuery("SHOW DATABASES");
  while($dao->fetch()){
    if($dao->Database != 'information_schema'){
      $databases[] = $dao->Database;
    }
  }
  $success = $converter->convert($databases);
}

/**
 * Allows for converting all databases to another charset.
 */
class CRMCharsetConverter {

  /**
   * Character set.
   * @var string
   */
  protected $charset = 'utf8mb4';

  /**
   * Collation.
   * @var string
   */
  protected $collation = 'utf8mb4_general_ci';

  /**
   * The current database connection for all actions.
   *
   * @var DatabaseConnection
   */
  protected $connection;

  public function __construct($charset = NULL, $collation = NULL) {
    if ($charset) {
      $this->charset = $charset;
    }
    if ($collation) {
      $this->collation = $collation;
    }
  }

  /**
   * Query the active connection, logging in verbose mode.
   *
   * The normal values are returned, use the connection directly if the return
   * value is needed.
   *
   * @param string $query
   *   The query to execute.
   * @param array $args
   *   An array of arguments for the prepared statement.
   *
   * @see DatabaseConnection::query()
   */
  public function query($query, array $args = array()) {
    $i = 0;
    foreach ($args as $str => $value) {
      $i++;
      $query = str_replace($str, '%'.$i , $query);
      if($str == ':comment'){
        $params[$i] = array($value, 'String');
      }elseif($str == ':default'){
        $params[$i] = $value;
      }else{
        $params[$i] = array($value, 'Text');
      }
    }
    $dao = CRM_Core_DAO::executeQuery($query, $params);
    $dao->free();
    CRM_Core_Error::debug_log_message('Executed query: ' . $query . ';');
  }

  /**
   * Convert the MySQL drupal databases character set and collation.
   *
   * @param array $databases
   *   The Drupal 7 database info array.
   */
  public function convert($databases) {

    civicrm_initialize();
    foreach ($databases as $database) {
      $this->convertDatabase($database);
      $this->convertTables($database);
    }

    // For each table in the database.
    $success = TRUE;
    CRM_Core_Error::debug_log_message('Finished converting the ' . $dao->Database . ' MySQL database!', 1);

    return $success;
  }

  /**
   * @param string
   *   Database name.
   * @param string $charset
   *   (Optional) The character set.
   * @param string $collation
   *   (Optional) The collation.
   *
   * @return bool
   *   success|failure.
   */
  public function convertDatabase($database_name, $charset = NULL, $collation = NULL) {
    CRM_Core_Error::debug_log_message('Converting database: ' . $database_name, 1);
    $sql = "ALTER DATABASE `" . $database_name . "` CHARACTER SET = :charset COLLATE = :collation;";

    return $this->query($sql, array(
      ':charset' => $charset ? $charset : $this->charset,
      ':collation' => $collation ? $collation : $this->collation,
    ));
  }

  /**
   * Converts all the tables defined by drupal_get_schema().
   *
   * @param string $charset
   *   (Optional) The character set.
   * @param string $collation
   *   (Optional) The collation.
   *
   * @return bool
   *   success|failure.
   */
  public function convertTables($database, $charset = NULL, $collation = NULL) {
    // For each table:
    // Deal only with Drupal managed tables.
    $dao = CRM_Core_DAO::executeQuery("SHOW TABLES");
    $inx = "Tables_in_".$database;
    while($dao->fetch()){
      $table_name = $dao->{$inx}; 
      if(strpos($table_name, 'civicrm') === 0){
        $this->convertTable($table_name, $charset, $collation);
      }
    }
  }

  /**
   * Converts a table to a desired character set and collation.
   *
   * @param string $table_name
   *  The database table name.
   * @param string $charset
   *   (Optional) The character set.
   * @param string $collation
   *   (Optional) The collation.
   *
   * @return bool
   *   success|failure.
   */
  public function convertTable($table_name, $charset = NULL, $collation = NULL) {
    $this->query("ALTER TABLE " . $table_name . " ROW_FORMAT=DYNAMIC ENGINE=INNODB");
    $sql = "ALTER TABLE " . $table_name . " CHARACTER SET = :charset COLLATE = :collation";
    CRM_Core_Error::debug_log_message('Converting table: ' . $table_name, 1);
    $result = $this->query($sql, array(
      ':charset' => $charset ? $charset : $this->charset,
      ':collation' => $collation ? $collation : $this->collation,
    ));
    $this->convertTableFields($table_name, $charset, $collation);
    $this->query("OPTIMIZE TABLE " . $table_name);
    return $result;
  }

  /**
   * Converts a table's field to a desired character set and collation.
   *
   * @param string $table_name
   *  The database table name.
   * @param string $charset
   *   (Optional) The character set.
   * @param string $collation
   *   (Optional) The collation.
   *
   * @return bool
   *   success|failure.
   */
  public function convertTableFields($table_name, $charset = NULL, $collation = NULL) {
    $row = CRM_Core_DAO::executeQuery("SHOW FULL FIELDS FROM " . $table_name);
    $charset = $charset ? $charset : $this->charset;
    $collation = $collation ? $collation : $this->collation;
    while($row->fetch()){
      // Skip fields that don't have collation, as they are probably int or similar.
      // or if we are using that collation for this field already save a query
      // or is not binary.
      if (!$row->Collation || $row->Collation === $collation) {
        continue;
      }
      // Skip fields that have non-utf8 collation.
      if (strpos($row->Collation, 'utf8') !== 0) {
        continue;
      }
      CRM_Core_Error::debug_log_message('Converting field: ' . $table_name . '.' . $row->Field, 1);

      // Detect the BINARY option from hook_schema.
      if (strpos($row->Collation, '_bin') !== FALSE) {
        $collation = 'utf8mb4_bin';
      }

      $default = '';
      if ($row->Default !== NULL) {
        $default = 'DEFAULT ' . ($row->Default == "CURRENT_TIMESTAMP" ? "CURRENT_TIMESTAMP" : ":default");
      }
      elseif ($row->Null == 'YES' && $row->Key == '') {
        if ($row->Type == 'timestamp') {
          $default = 'NULL ';
        }
        $default .= 'DEFAULT NULL';
      }

      $sql = "ALTER TABLE " . $table_name . "
              MODIFY `" . $row->Field . "` " .
              $row->Type . " " .
              "CHARACTER SET :charset COLLATE :collation" . " " .
              ($row->Null == "YES" ? "" : "NOT NULL ") .
              $default . " " .
              $row->Extra . " " .
              "COMMENT :comment";

      $params = array(
        ':charset' => $charset,
        ':collation' => $collation,
        ':comment' => $row->Comment,
      );
      if (strstr($default, ':default')) {
        if($row->Default == "CURRENT_TIMESTAMP" ){
          $params[':default'] = array("CURRENT_TIMESTAMP", "Text");
        }elseif(CRM_Utils_Rule::numeric($row->Default)){
          $params[':default'] = array($row->Default, "Int");
        }else{
          $params[':default'] = array($row->Default, "String");
        }
        
      }
      $this->query($sql, $params);
    }
    $row->free();
  }
}