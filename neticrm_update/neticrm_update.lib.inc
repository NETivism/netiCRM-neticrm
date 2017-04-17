<?php
// From https://fosswiki.liip.ch/display/DRUPAL/Script+creation+of+languages+and+import+po+files
/**
 * Add translations from a PO file.
 * This will populate the locales_target table in the database
 * with the strings from the PO file.
 *
 * @param $poFile string full path to .po file or module name
 * @param $langcode string (e.g. fr, de, it)
 * @param $mode const LOCALE_IMPORT_OVERWRITE (overwrite existing translations) or LOCALE_IMPORT_KEEP (only import new strings)
 * @param $group string Text group to import PO file into (eg. 'default' for interface translations)
 * @param $add_language_options array optional parameters for locale_add_language (name, native, direction, domain, prefix, enabled, default)
 * @return bool success
 */
function neticrm_update_import_po($pofile, $langcode, $mode = LOCALE_IMPORT_OVERWRITE, $group = 'default') {
  require_once DRUPAL_ROOT . '/includes/locale.inc';
  if (module_exists($pofile)) {
		$uri = drupal_get_path("module", $module)."/translations/$langcode.po"; 
  }
  else {
    $uri = $pofile;
  }
  
  $file = new stdClass;
  $file->filepath = $uri;
  $file->uri = $uri;
  $file->filename = basename($uri);
  return _locale_import_po($file, $langcode, $mode, $group);
}
 
/**
 * Find all po files under a given path
 *
 * @param $path string
 * @param $recurse boolean
 * @return array of string paths to po files
 */
function neticrm_update_find_po_files($path, $recurse=TRUE) {
  $path = rtrim($path, '/') . '/*';
  $pattern = '/\.po$/';
  $files = array();
  foreach (glob ($path) as $fullname) {
    if ($recurse && is_dir($fullname)) {
      $files = array_merge($files, neticrm_update_find_po_files($fullname, $recurse));
    } else if (preg_match($pattern, $fullname)) {
      $files[] = $fullname;
    }
  }
  return $files;
}

/**
 * Add custom search if name not exists.
 *
 * @param $name string
 * @param $desc string
 * return integer id of custom search
 */
function neticrm_update_add_custom_search($name, $desc){
  $find = array(
    1 => array($name, 'String')
  );
  $exists = CRM_Core_DAO::singleValueQuery("SELECT value FROM civicrm_option_value WHERE name LIKE %1", $find);

  if(!$exists){
    $params = array(
      'label' => $name,
      'name' => $name,
      'description' => t($desc),
      'is_active' => 1,
    );
    $group_params = array('name' => 'custom_search');
    $action = CRM_Core_Action::ADD;
    $custom_search_id = NULL;
    $custom_search = CRM_Core_OptionValue::addOptionValue($params, $group_params, $action, $custom_search_id);
    return $custom_search->value;
  }
  else{
    return $exists;
  }
}

/**
 * Add new civicrm report template and instance
 * 
 * @param $label string
 * @param $desc string
 * @param $url  string
 * @param $class string
 * @param $component_id integer 
 * return integer instance_id
 */
function neticrm_update_add_report($label, $desc, $url, $class, $component_id){
  $group_params = array('name' => ('report_template'));
  require_once 'CRM/Core/OptionValue.php';

  $exists = CRM_Core_OptionGroup::getRowValues('report_template', $class, 'name', 'String', FALSE);
  if(!empty($exists)){
    $action = CRM_Core_Action::UPDATE;
    $id = $exists['id'];
  }
  else{
    $action = CRM_Core_Action::ADD;
    $id = NULL;
  }
  
  $params = array(
    'label' => $label,
    'description' => $desc,
    'value' => $url,
    'name' => $class,
    'component_id' => $component_id,
    'is_active' => 1,
    'weight' => 50,
  );
  $added = CRM_Core_OptionValue::addOptionValue($params, $group_params, $action, $id);
  return $added;
}
