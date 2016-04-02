<?php
/**
 * @file
 * Update for 7209
 *
 * This file should be civicrm compliant.
 */
function _neticrm_update_update_7209(){
  $title = '參加活動最多的人';
  $desc = '顯示參加活動最多的人報表';
  $url = 'contact/topparticipant';
  $class= 'CRM_Report_Form_Contact_Participate';

  $result = neticrm_update_add_report($title, $desc, $url, $class, NULL);
  if($result->id){
    // add instance
    $sql = <<<EOD
INSERT INTO `civicrm_report_instance` ( `domain_id`, `title`, `report_id`, `description`, `permission`, `form_values`)
VALUES ( 1, '參加活動最多的人', 'contact/topparticipant', '顯示參加活動最多的人報表', 'access CiviEvent', 'a:34:{s:6:"fields";a:3:{s:9:"sort_name";s:1:"1";s:2:"id";s:1:"1";s:9:"status_id";s:1:"1";}s:12:"sort_name_op";s:3:"has";s:15:"sort_name_value";s:0:"";s:8:"email_op";s:3:"has";s:11:"email_value";s:0:"";s:21:"participant_count_min";s:0:"";s:21:"participant_count_max";s:0:"";s:20:"participant_count_op";s:3:"gte";s:23:"participant_count_value";s:0:"";s:11:"event_id_op";s:2:"in";s:14:"event_id_value";a:0:{}s:6:"sid_op";s:2:"in";s:9:"sid_value";a:0:{}s:6:"rid_op";s:2:"in";s:9:"rid_value";a:0:{}s:34:"participant_register_date_relative";s:1:"0";s:30:"participant_register_date_from";s:0:"";s:28:"participant_register_date_to";s:0:"";s:6:"eid_op";s:2:"in";s:9:"eid_value";a:0:{}s:25:"event_start_date_relative";s:1:"0";s:21:"event_start_date_from";s:0:"";s:19:"event_start_date_to";s:0:"";s:23:"event_end_date_relative";s:1:"0";s:19:"event_end_date_from";s:0:"";s:17:"event_end_date_to";s:0:"";s:11:"description";s:33:"Report for list top participants.";s:13:"email_subject";s:0:"";s:8:"email_to";s:0:"";s:8:"email_cc";s:0:"";s:10:"permission";s:1:"0";s:9:"parent_id";s:0:"";s:6:"groups";s:0:"";s:9:"domain_id";i:1;}');
EOD;
    CRM_Core_DAO::executeQuery($sql);
    return 'Successful added report';
  }
  else{
    return FALSE;
  }
}
