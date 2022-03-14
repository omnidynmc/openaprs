ALTER TABLE `igated` ADD `callsign_id` INT NOT NULL AFTER `packet_id` , ADD INDEX ( `callsign_id` );
ALTER TABLE `lastraw` ADD `callsign_id` INT NOT NULL AFTER `packet_id`;
ALTER TABLE `lastraw` ADD INDEX ( `callsign_id` ) 
ALTER TABLE `lastposition` ADD `callsign_id` INT NOT NULL AFTER `packet_id`;
ALTER TABLE `lastposition` ADD INDEX ( `callsign_id` ) 
ALTER TABLE `laststatus` ADD `callsign_id` INT NOT NULL AFTER `packet_id`;
ALTER TABLE `laststatus` ADD INDEX ( `callsign_id` );
ALTER TABLE `lastweather` ADD `callsign_id` INT NOT NULL AFTER `packet_id`;
ALTER TABLE `lastweather` ADD INDEX ( `callsign_id` );
ALTER TABLE `cache_24hr` ADD `callsign_id` INT NOT NULL AFTER `packet_id`;
ALTER TABLE `cache_24hr` ADD INDEX ( `callsign_id` );
ALTER TABLE `last_bulletin` ADD `callsign_id` INT NOT NULL AFTER `packet_id` , ADD INDEX ( `callsign_id` );
ALTER TABLE `last_frequency` ADD `callsign_id` INT NOT NULL AFTER `packet_id` , ADD INDEX ( `callsign_id` );
ALTER TABLE `last_message` ADD `callsign_id` INT NOT NULL AFTER `packet_id` , ADD INDEX ( `callsign_id` );
ALTER TABLE `last_object` ADD `callsign_id` INT NOT NULL AFTER `packet_id` , ADD INDEX ( `callsign_id` );
ALTER TABLE `last_telemetry` ADD `callsign_id` INT NOT NULL AFTER `packet_id` , ADD INDEX ( `callsign_id` );
ALTER TABLE `messages` ADD `callsign_id` INT NOT NULL AFTER `packet_id` , ADD INDEX ( `callsign_id` ) ;
ALTER TABLE `position` ADD `callsign_id` INT NOT NULL AFTER `packet_id` , ADD INDEX ( `callsign_id` ) ;
ALTER TABLE `raw` ADD `callsign_id` INT NOT NULL AFTER `packet_id` , ADD INDEX ( `callsign_id` ) ;
ALTER TABLE `raw_errors` ADD `callsign_id` INT NOT NULL AFTER `packet_id` , ADD INDEX ( `callsign_id` ) ;
ALTER TABLE `status` ADD `callsign_id` INT NOT NULL AFTER `packet_id` , ADD INDEX ( `callsign_id` ) ;
ALTER TABLE `telemetry` ADD `callsign_id` INT NOT NULL AFTER `packet_id` , ADD INDEX ( `callsign_id` ) ;
ALTER TABLE `telemetry_bits` ADD `callsign_id` INT NOT NULL AFTER `packet_id` , ADD INDEX ( `callsign_id` ) ;
ALTER TABLE `telemetry_eqns` ADD `callsign_id` INT NOT NULL AFTER `packet_id` , ADD INDEX ( `callsign_id` ) ;
ALTER TABLE `telemetry_parm` ADD `callsign_id` INT NOT NULL AFTER `packet_id` , ADD INDEX ( `callsign_id` ) ;
ALTER TABLE `telemetry_unit` ADD `callsign_id` INT NOT NULL AFTER `packet_id` , ADD INDEX ( `callsign_id` ) ;
ALTER TABLE `weather` ADD `callsign_id` INT NOT NULL AFTER `packet_id` , ADD INDEX ( `callsign_id` ) ;

CREATE TABLE IF NOT EXISTS `callsign` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `source` varchar(9) CHARACTER SET utf8 NOT NULL DEFAULT '',
  `created` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `create_ts` int(11) NOT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `source` (`source`)
) ENGINE=MyISAM  DEFAULT CHARSET=utf8 AUTO_INCREMENT=13391 ;

INSERT IGNORE INTO callsign (source) SELECT DISTINCT UPPER(source) FROM lastposition;
INSERT IGNORE INTO callsign (source) SELECT DISTINCT UPPER(source) FROM last_message;
INSERT IGNORE INTO callsign (source) SELECT DISTINCT UPPER(source) FROM lastraw;
INSERT IGNORE INTO callsign (source) SELECT DISTINCT UPPER(source) FROM lastweather;
INSERT IGNORE INTO callsign (source) SELECT DISTINCT UPPER(source) FROM last_frequency;
INSERT IGNORE INTO callsign (source) SELECT DISTINCT UPPER(source) FROM last_bulletin;
INSERT IGNORE INTO callsign (source) SELECT DISTINCT UPPER(source) FROM last_object;
INSERT IGNORE INTO callsign (source) SELECT DISTINCT UPPER(source) FROM last_telemetry;
INSERT IGNORE INTO callsign (source) SELECT DISTINCT UPPER(source) FROM laststatus;

CREATE TABLE IF NOT EXISTS `packet` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `callsign_id` int(11) NOT NULL,
  `packet_date` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `create_ts` int(11) NOT NULL,
  PRIMARY KEY (`id`, packet_date),
  KEY `callsign_id` (`callsign_id`)
) ENGINE=MyISAM  DEFAULT CHARSET=utf8 AUTO_INCREMENT=1;

  ALTER TABLE packet PARTITION BY RANGE (TO_DAYS(packet_date)) (
partition 2011_2_1 values less than (to_days('2011-2-1')),
partition 2011_2_2 values less than (to_days('2011-2-2')),
partition 2011_2_3 values less than (to_days('2011-2-3')),
partition 2011_2_4 values less than (to_days('2011-2-4')),
partition 2011_2_5 values less than (to_days('2011-2-5')),
partition 2011_2_6 values less than (to_days('2011-2-6')),
partition 2011_2_7 values less than (to_days('2011-2-7')),
partition 2011_2_8 values less than (to_days('2011-2-8')),
partition 2011_2_9 values less than (to_days('2011-2-9')),
partition 2011_2_10 values less than (to_days('2011-2-10')),
partition 2011_2_11 values less than (to_days('2011-2-11')),
partition 2011_2_12 values less than (to_days('2011-2-12')),
partition 2011_2_13 values less than (to_days('2011-2-13')),
partition 2011_2_14 values less than (to_days('2011-2-14')),
partition 2011_2_15 values less than (to_days('2011-2-15')),
partition 2011_2_16 values less than (to_days('2011-2-16')),
partition 2011_2_17 values less than (to_days('2011-2-17')),
partition 2011_2_18 values less than (to_days('2011-2-18')),
partition 2011_2_19 values less than (to_days('2011-2-19')),
partition 2011_2_20 values less than (to_days('2011-2-20')),
partition 2011_2_21 values less than (to_days('2011-2-21')),
partition 2011_2_22 values less than (to_days('2011-2-22')),
partition 2011_2_23 values less than (to_days('2011-2-23')),
partition 2011_2_24 values less than (to_days('2011-2-24')),
partition 2011_2_25 values less than (to_days('2011-2-25')),
partition 2011_2_26 values less than (to_days('2011-2-26')),
partition 2011_2_27 values less than (to_days('2011-2-27'))
);


ALTER TABLE `raw`
  DROP `ip_source`,
  DROP `source`,
  DROP `destination`,
  DROP `digipeater_0`,
  DROP `digipeater_1`,
  DROP `digipeater_2`,
  DROP `digipeater_3`,
  DROP `digipeater_4`,
  DROP `digipeater_5`,
  DROP `digipeater_6`,
  DROP `digipeater_7`,
  DROP `hostid`;

ALTER TABLE `raw` DROP `packet_time` ;

CREATE TABLE IF NOT EXISTS `raw_meta` (
  `packet_id` char(32) CHARACTER SET latin1 NOT NULL,
  `callsign_id` int(11) NOT NULL,
  `packet_time` int(10) unsigned NOT NULL DEFAULT '0',
  `packet_date` datetime NOT NULL DEFAULT '0000-00-00 00:00:00',
  `ip_source` varchar(255) CHARACTER SET latin1 DEFAULT NULL,
  `source` varchar(9) CHARACTER SET latin1 NOT NULL DEFAULT '',
  `destination` varchar(9) CHARACTER SET latin1 NOT NULL DEFAULT '',
  `digipeater_0` varchar(9) CHARACTER SET latin1 DEFAULT NULL,
  `digipeater_1` varchar(9) CHARACTER SET latin1 DEFAULT NULL,
  `digipeater_2` varchar(9) CHARACTER SET latin1 DEFAULT NULL,
  `digipeater_3` varchar(9) CHARACTER SET latin1 DEFAULT NULL,
  `digipeater_4` varchar(9) CHARACTER SET latin1 DEFAULT NULL,
  `digipeater_5` varchar(9) CHARACTER SET latin1 DEFAULT NULL,
  `digipeater_6` varchar(9) CHARACTER SET latin1 DEFAULT NULL,
  `digipeater_7` varchar(9) CHARACTER SET latin1 DEFAULT NULL,
  `hostid` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `create_ts` int(11) NOT NULL DEFAULT '0',
  KEY `source` (`source`),
  KEY `create_ts` (`create_ts`),
  KEY `packet_date` (`packet_date`),
  KEY `callsign_id` (`callsign_id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8
/*!50100 PARTITION BY RANGE (TO_DAYS(packet_date))
(PARTITION 2011_04_06 VALUES LESS THAN (734599) ENGINE = MyISAM,
 PARTITION 2011_04_07 VALUES LESS THAN (734600) ENGINE = MyISAM,
 PARTITION 2011_04_08 VALUES LESS THAN (734601) ENGINE = MyISAM,
 PARTITION 2011_04_09 VALUES LESS THAN (734602) ENGINE = MyISAM,
 PARTITION 2011_04_10 VALUES LESS THAN (734603) ENGINE = MyISAM,
 PARTITION 2011_04_11 VALUES LESS THAN (734604) ENGINE = MyISAM,
 PARTITION 2011_04_12 VALUES LESS THAN (734605) ENGINE = MyISAM,
 PARTITION 2011_04_13 VALUES LESS THAN (734606) ENGINE = MyISAM,
 PARTITION 2011_04_14 VALUES LESS THAN (734607) ENGINE = MyISAM,
 PARTITION 2011_04_15 VALUES LESS THAN (734608) ENGINE = MyISAM,
 PARTITION 2011_04_16 VALUES LESS THAN (734609) ENGINE = MyISAM) */;

CREATE TABLE IF NOT EXISTS `weather_rollup` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `packet_id` char(32) CHARACTER SET latin1 NOT NULL,
  `callsign_id` int(11) NOT NULL,
  `source` char(9) CHARACTER SET latin1 NOT NULL DEFAULT '',
  `packet_date` datetime NOT NULL DEFAULT '0000-00-00 00:00:00',
  `wind_direction` smallint(3) unsigned DEFAULT NULL,
  `wind_speed` tinyint(4) unsigned DEFAULT NULL,
  `wind_gust` tinyint(4) unsigned DEFAULT NULL,
  `wind_sustained` tinyint(4) unsigned DEFAULT NULL,
  `temperature` tinyint(4) DEFAULT NULL,
  `rain_hour` float DEFAULT NULL,
  `rain_calendar_day` float DEFAULT NULL,
  `rain_24hour_day` float DEFAULT NULL,
  `humidity` tinyint(4) unsigned DEFAULT NULL,
  `barometer` double(6,2) DEFAULT NULL,
  `luminosity` smallint(5) unsigned DEFAULT NULL,
  `create_ts` int(14) NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`),
  KEY `source` (`source`),
  KEY `create_ts` (`create_ts`),
  KEY `source_ts` (`source`,`create_ts`),
  KEY `callsign_id` (`callsign_id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 AUTO_INCREMENT=1 ;

CREATE TABLE IF NOT EXISTS `weather_meta` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `callsign_id` int(11) NOT NULL,
  `last_rollup` timestamp NOT NULL DEFAULT '0000-00-00 00:00:00',
  `created` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `create_ts` int(11) NOT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `callsign_id` (`callsign_id`),
  KEY `last_rollup` (`last_rollup`),
  KEY `callsign_rollup_idx` (`callsign_id`,`last_rollup`)
) ENGINE=MyISAM  DEFAULT CHARSET=latin1 AUTO_INCREMENT=1 ;

ALTER TABLE `weather_rollup` ADD `num_rolled` INT NOT NULL AFTER `callsign_id` ;
