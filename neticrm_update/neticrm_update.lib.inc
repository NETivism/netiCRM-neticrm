<?php
// From https://fosswiki.liip.ch/display/DRUPAL/Script+creation+of+languages+and+import+po+files
/**
 * Add translations from a PO file.
 * This will populate the locales_target table in the database
 * with the strings from the PO file.
 *
 * @param $langcode string (e.g. fr, de, it)
 * @param $poFile string full path to .po file
 * @param $mode const LOCALE_IMPORT_OVERWRITE (overwrite existing translations) or LOCALE_IMPORT_KEEP (only import new strings)
 * @param $group string Text group to import PO file into (eg. 'default' for interface translations)
 * @param $add_language_options array optional parameters for locale_add_language (name, native, direction, domain, prefix, enabled, default)
 * @return bool success
 */
function neticrm_update_import_po($pofile, $langcode, $mode = LOCALE_IMPORT_OVERWRITE, $group = 'default') {
  $file = new stdClass;
  $file->filepath = $pofile;
  $file->filename = basename($pofile);
  $file->uri = $file->filepath;
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
