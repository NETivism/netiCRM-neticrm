<?php

/**
 * Example function to doing database update
 *
 * @return mixed Success: NULL, TRUE, String. Failed: FALSE or string contain "[error]"
 */
function _neticrm_update_update_x309() {
  $hasOpenssl = CRM_Utils_Crypt::checkAvailableCrypt('openssl');
  $hasMcrypt = CRM_Utils_Crypt::checkAvailableCrypt('mcrypt');
  // doing migration
  if ($hasOpenssl && $hasMcrypt) {
    $dao = CRM_Core_DAO::executeQuery("SELECT id, mailing_backend FROM civicrm_preferences WHERE mailing_backend IS NOT NULL");
    while($dao->fetch()) {
      if (!empty($dao->mailing_backend)) {
        $setting = unserialize($dao->mailing_backend);
        if (!empty($setting['smtpPassword'])) {
          $string_encrypted = $setting['smtpPassword'];
          if (substr($string_encrypted, 0, CRM_Utils_Crypt::VER2) !== CRM_Utils_Crypt::VER2) {
            $string_decrypted = CRM_Utils_Crypt::deprecatedDecrypt($string_encrypted);
            if (!empty($string_decrypted) && $string_encrypted != $string_decrypted) {
              $setting['smtpPassword'] = CRM_Utils_Crypt::encrypt($string_decrypted);
              $mailing_backend = serialize($setting);
              CRM_Core_DAO::setFieldValue('CRM_Core_DAO_Preferences', $dao->id, 'mailing_backend', $mailing_backend);
            }
          }
        }
      }
    }
    return "Successful migrate passwords to openssl encrypted";
  }
  else {
    return "[error] Mcrypt and openssl not both available. Do not migrate password encryption";
  }
}
