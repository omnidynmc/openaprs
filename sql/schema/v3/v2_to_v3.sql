CREATE TABLE IF NOT EXISTS `statuses` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `hash` char(32) CHARACTER SET utf8 NOT NULL,
  `body` tinytext CHARACTER SET utf8 NOT NULL,
  `created` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`id`),
  UNIQUE KEY `hash` (`hash`),
  KEY `created` (`created`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 AUTO_INCREMENT=1 ;

CREATE TABLE IF NOT EXISTS `path` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `hash` char(32) NOT NULL,
  `body` varchar(128) NOT NULL,
  `packet_date` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `create_ts` int(11) NOT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `hash` (`hash`),
  KEY `callsign_id` (`body`),
  KEY `packet_date` (`packet_date`)
) ENGINE=MyISAM  DEFAULT CHARSET=utf8 AUTO_INCREMENT=1 ;

DROP TABLE `cache_30m`;

ALTER TABLE `lastposition` ADD `status_id` INT NOT NULL DEFAULT '0' AFTER `status` ;

ALTER TABLE `cache_24hr` ADD `path_id` INT NOT NULL AFTER `path`;
ALTER TABLE `position` ADD `path_id` INT NOT NULL AFTER `path`;
ALTER TABLE `lastposition` ADD `path_id` INT NOT NULL AFTER `path`; 
ALTER TABLE `raw_meta` DROP `source`;
ALTER TABLE `position` ADD `status_id` INT NOT NULL DEFAULT '0' AFTER `path_id` ;
ALTER TABLE `cache_24hr` ADD `status_id` INT NOT NULL DEFAULT '0' AFTER `path_id` ;

ALTER TABLE `lastposition` DROP `path`;
ALTER TABLE `position` DROP `path`;
ALTER TABLE `cache_24hr` DROP `path`;

DROP TABLE `msg2email_snoop_users`;
DROP TABLE `msg2email_users` ;

DROP TABLE `fd_area`;
DROP TABLE `fd_band` ;
DROP TABLE `fd_club` ;
DROP TABLE `fd_event` ;
DROP TABLE `fd_mode` ;
DROP TABLE `fd_op` ;
DROP TABLE `fd_section` ;
DROP TABLE `fd_table` ;
DROP TABLE `qa_hosts` ;

ALTER TABLE `packet` CHANGE `id` `id` INT( 11 ) UNSIGNED NOT NULL AUTO_INCREMENT ;
ALTER TABLE `lastposition` CHANGE `packet_id` `packet_id` INT( 11 ) UNSIGNED NOT NULL ;
ALTER TABLE `position` CHANGE `packet_id` `packet_id` INT( 11 ) UNSIGNED NOT NULL ;

ALTER TABLE `lastposition` DROP `source` ;
ALTER TABLE `lastraw` DROP `source` ;
ALTER TABLE `laststatus` DROP `source` ;
ALTER TABLE `lastweather` DROP `source` ;
ALTER TABLE `last_bulletin` DROP `source` ;
ALTER TABLE `last_frequency` DROP `source` ;
ALTER TABLE `last_message` DROP `source` ;
ALTER TABLE `last_object` DROP `source` ;
ALTER TABLE `last_source` DROP `source` ;
ALTER TABLE `messages` DROP `source` ;
ALTER TABLE `position` DROP `source` ;
ALTER TABLE `status` DROP `source` ;
ALTER TABLE `telemetry` DROP `source` ;
ALTER TABLE `telemetry_bits` DROP `source` ;
ALTER TABLE `telemetry_eqns` DROP `source` ;
ALTER TABLE `telemetry_parm` DROP `source` ;
ALTER TABLE `telemetry_unit` DROP `source` ;

DELETE FROM t1 USING lastposition t1, lastposition t2 WHERE (NOT t1.packet_id=t2.packet_id) 
AND (t1.callsign_id=t2.callsign_id)

DELETE FROM t1 USING lastweather t1, lastweather t2 WHERE (NOT t1.packet_id=t2.packet_id) 
AND (t1.callsign_id=t2.callsign_id);
ALTER TABLE `openaprs`.`lastweather` DROP PRIMARY KEY , ADD PRIMARY KEY ( `callsign_id` ) ;

DELETE FROM t1 USING lastraw t1, lastraw t2 WHERE (NOT t1.packet_id=t2.packet_id) 
AND (t1.callsign_id=t2.callsign_id);
ALTER TABLE `openaprs`.`lastraw` DROP PRIMARY KEY , ADD PRIMARY KEY ( `callsign_id` ) ;

DELETE FROM t1 USING laststatus t1, laststatus t2 WHERE (NOT t1.packet_id=t2.packet_id) 
AND (t1.callsign_id=t2.callsign_id);
ALTER TABLE `openaprs`.`laststatus` DROP PRIMARY KEY , ADD PRIMARY KEY ( `callsign_id` ) ;

DELETE FROM t1 USING last_message t1, last_message t2 WHERE (NOT t1.packet_id=t2.packet_id) 
AND (t1.callsign_id=t2.callsign_id);
ALTER TABLE `openaprs`.`last_message` DROP PRIMARY KEY , ADD PRIMARY KEY ( `callsign_id` ) ;

DELETE FROM t1 USING last_bulletin t1, last_bulletin t2 WHERE (NOT t1.packet_id=t2.packet_id) 
AND (t1.callsign_id=t2.callsign_id);
ALTER TABLE `openaprs`.`last_bulletin` DROP PRIMARY KEY , ADD PRIMARY KEY ( `callsign_id` ) ;

DELETE FROM t1 USING last_frequency t1, last_frequency t2 WHERE (NOT t1.packet_id=t2.packet_id) 
AND (t1.callsign_id=t2.callsign_id);
ALTER TABLE `openaprs`.`last_frequency` DROP PRIMARY KEY , ADD PRIMARY KEY ( `callsign_id` ) ;

DELETE FROM t1 USING last_object t1, last_object t2 WHERE (NOT t1.packet_id=t2.packet_id) 
AND (t1.callsign_id=t2.callsign_id);
ALTER TABLE `openaprs`.`last_object` DROP PRIMARY KEY , ADD PRIMARY KEY ( `callsign_id` ) ;

DELETE FROM t1 USING last_telemetry t1, last_telemetry t2 WHERE (NOT t1.packet_id=t2.packet_id) 
AND (t1.callsign_id=t2.callsign_id);
ALTER TABLE `openaprs`.`last_telemetry` DROP PRIMARY KEY , ADD PRIMARY KEY ( `callsign_id` ) ;

ALTER TABLE `lastposition` DROP `source` ;
ALTER TABLE `laststatus` DROP `source` ;
ALTER TABLE `lastraw` DROP `source` ;
ALTER TABLE `lastweather` DROP `source` ;
ALTER TABLE `last_bulletin` DROP `source` ;
ALTER TABLE `last_frequency` DROP `source` ;
ALTER TABLE `last_message` DROP `source` ;
ALTER TABLE `last_object` DROP `source` ;
ALTER TABLE `last_telemetry` DROP `source` ;
ALTER TABLE `position` DROP `source` ;
ALTER TABLE `weather` DROP `source` ;
ALTER TABLE `raw` DROP `source` ;
ALTER TABLE `cache_24hr` DROP `source` ;

ALTER TABLE `last_bulletin` CHANGE `packet_id` `packet_id` BIGINT NOT NULL ;
ALTER TABLE `last_frequency` CHANGE `packet_id` `packet_id` BIGINT NOT NULL ;
ALTER TABLE `last_message` CHANGE `packet_id` `packet_id` BIGINT NOT NULL ;
ALTER TABLE `last_object` CHANGE `packet_id` `packet_id` BIGINT NOT NULL ;
ALTER TABLE `last_telemetry` CHANGE `packet_id` `packet_id` BIGINT NOT NULL ;
ALTER TABLE `lastposition` CHANGE `packet_id` `packet_id` BIGINT NOT NULL ;
ALTER TABLE `laststatus` CHANGE `packet_id` `packet_id` BIGINT NOT NULL ;
ALTER TABLE `lastweather` CHANGE `packet_id` `packet_id` BIGINT NOT NULL ;
ALTER TABLE `lastraw` CHANGE `packet_id` `packet_id` BIGINT NOT NULL ;

ALTER TABLE `openaprs`.`lastposition` ADD INDEX `lat_lng_create_idx` ( `latitude` , `longitude` , `create_ts` ) ;

ALTER TABLE `lastposition` ADD `packet_date` TIMESTAMP NOT NULL AFTER `callsign_id` ;
