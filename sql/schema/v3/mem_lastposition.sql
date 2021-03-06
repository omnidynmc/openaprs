CREATE TABLE IF NOT EXISTS `mem_lastposition` (
  `packet_id` bigint(20) NOT NULL,
  `callsign_id` int(11) NOT NULL,
  `packet_date` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  `name` varchar(9) CHARACTER SET latin1 NOT NULL DEFAULT '',
  `path_id` int(11) NOT NULL,
  `latitude` double(8,6) NOT NULL DEFAULT '0.000000',
  `longitude` double(9,6) NOT NULL DEFAULT '0.000000',
  `course` smallint(5) unsigned DEFAULT NULL,
  `symbol_table` char(1) CHARACTER SET latin1 DEFAULT NULL,
  `symbol_code` char(1) CHARACTER SET latin1 DEFAULT NULL,
  `phg_range` double(10,2) DEFAULT NULL,
  `phg_direction` int(11) DEFAULT NULL,
  `type` enum('P','O','I') CHARACTER SET latin1 NOT NULL DEFAULT 'P',
  PRIMARY KEY (`name`,`callsign_id`),
  KEY `latitude` (`latitude`),
  KEY `longitude` (`longitude`),
  KEY `packet_id` (`packet_id`),
  KEY `symbol_table` (`symbol_table`,`symbol_code`),
  KEY `callsign_id` (`callsign_id`),
  KEY `lat_lng_create_idx` (`latitude`,`longitude`),
  KEY `packet_date` (`packet_date`),
  KEY `lat_lng_pdate_idx` (`latitude`,`longitude`,`packet_date`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;
