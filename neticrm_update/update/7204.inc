<?php
/**
 * @file
 * Update for 7204
 *
 * This file should be civicrm compliant.
 */
function _neticrm_update_update_7204(){
  $replace = array(
    'receipt_text|htmlize' => array(
      ts('Contributions - Receipt (off-line)'),
      ts('Contributions - Receipt (on-line)'),
      ts('Memberships - Receipt (on-line)'),
    ),
    'confirm_email_text|htmlize' => array(
      ts('Events - Registration Confirmation and Receipt (off-line)'),
      ts('Events - Registration Confirmation and Receipt (on-line)'),
    ),
  );

  foreach($replace as $string => $templates){
    $string_new = str_replace('|htmlize', '', $string);
    foreach($templates as $t) {
      $params = array(1 => array($t, 'String'));
      $query = CRM_Core_DAO::executeQuery("SELECT id FROM civicrm_msg_template WHERE msg_title = %1", $params);
      $msgids = array();
      while($query->fetch()){
        $msgids[] = $query->id;
      }
      $params = array(
        1 => array($string, 'String'),
        2 => array($string_new, 'String'),
      );
      if(!empty($msgids)){
        $msgidstring = implode(',', $msgids); 
        CRM_Core_DAO::executeQuery("UPDATE civicrm_msg_template SET msg_html = REPLACE(`msg_html`, %1, %2) WHERE id IN($msgidstring)", $params);
      }
    }
  }
  return "Successful update message templates wrong htmlize. (#16961)";
}
