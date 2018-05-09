<?php

function _neticrm_update_update_7244(){
  $sql = "
CREATE TABLE civicrm_track (
	 session_key varchar(255) NOT NULL COMMENT 'Session of this visit',
   visit_date datetime NOT NULL   COMMENT 'Date time of this visit',
	 page_type varchar(64) NOT NULL   COMMENT 'Page type like civicrm_event or civicrm_contribution_page',
	 page_id int unsigned NOT NULL   COMMENT 'Page id.',
	 entity_table varchar(64)    COMMENT 'Entity table like civicrm_contribution or civicrm_participant or civicrm_membership',
	 entity_id int unsigned    COMMENT 'Entity id like contribution id or participant id',
	 state int unsigned    COMMENT 'Depth of this visit',
	 referrer_type varchar(64)    COMMENT 'Referrer type of inbound visit',
	 referrer_network varchar(64)    COMMENT 'Referrer network of inbound visit',
	 referrer_url varchar(255)    COMMENT 'Referrer url of inbound traffic. Null when no referrer',
	 landing varchar(255)    COMMENT 'Landing page of this record without protocol and host name',
	 utm_source varchar(255),
	 utm_medium varchar(255),
	 utm_campaign varchar(255),
	 utm_term varchar(255),
	 utm_content varchar(255),
   PRIMARY KEY ( session_key ),
   INDEX index_page_type_id(page_type, page_id),
   INDEX index_entity_table_id(entity_table,entity_id),
   INDEX index_state(state),
   INDEX index_referrer_type(referrer_type),
   INDEX index_referrer_network(referrer_network),
   INDEX index_utm_source(utm_source),
   INDEX index_utm_medium(utm_medium)
) ENGINE=InnoDB DEFAULT CHARACTER SET utf8 COLLATE utf8_unicode_ci ROW_FORMAT=DYNAMIC
";
  CRM_Core_DAO::executeQuery($sql);
  return 'Successful add civicrm_track table';
}
