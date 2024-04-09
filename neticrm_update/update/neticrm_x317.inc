<?php
function _neticrm_update_update_x317() {
    // add qpdf options
    civicrm_initialize();
    $params = array(
    'qpdfPath' => '/usr/bin/qpdf',
    );
    CRM_Core_BAO_ConfigSetting::add($params);
}