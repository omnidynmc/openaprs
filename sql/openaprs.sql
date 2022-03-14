-- MySQL dump 10.13  Distrib 5.1.63, for debian-linux-gnu (x86_64)
--
-- Host: localhost    Database: openaprs
-- ------------------------------------------------------
-- Server version	5.1.63-rel13.4-log

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;

--
-- Table structure for table `accounting_finance`
--

DROP TABLE IF EXISTS `accounting_finance`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `accounting_finance` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `datestamp` date NOT NULL,
  `trans` enum('inc','exp') NOT NULL,
  `amount` float(6,2) NOT NULL,
  `desc` varchar(128) NOT NULL,
  `create_ts` int(11) NOT NULL,
  PRIMARY KEY (`id`),
  KEY `trans` (`trans`)
) ENGINE=InnoDB AUTO_INCREMENT=1882 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `apns_feedback`
--

DROP TABLE IF EXISTS `apns_feedback`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `apns_feedback` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `apns_timestamp` int(11) NOT NULL,
  `device_token` varchar(128) NOT NULL,
  `processed_ts` int(11) NOT NULL DEFAULT '0',
  `create_ts` int(11) NOT NULL,
  PRIMARY KEY (`id`),
  KEY `processed_ts` (`processed_ts`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `apns_push`
--

DROP TABLE IF EXISTS `apns_push`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `apns_push` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `apns_register_id` int(11) NOT NULL,
  `badge` int(1) NOT NULL,
  `alertmsg` varchar(255) NOT NULL,
  `sent` enum('Y','N') NOT NULL DEFAULT 'N',
  `sent_ts` int(11) NOT NULL DEFAULT '0',
  `create_ts` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=1728738 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `apns_register`
--

DROP TABLE IF EXISTS `apns_register`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `apns_register` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `user_id` int(11) NOT NULL,
  `callsign` varchar(9) NOT NULL,
  `uid` varchar(128) NOT NULL,
  `device_token` varchar(128) NOT NULL,
  `environment` enum('devel','prod') NOT NULL DEFAULT 'prod',
  `active` enum('Y','N') NOT NULL DEFAULT 'Y',
  `check_ts` int(11) NOT NULL,
  `update_ts` int(11) NOT NULL,
  `create_ts` int(11) NOT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `deviceid` (`device_token`),
  KEY `callsign` (`callsign`),
  KEY `user_id` (`user_id`),
  KEY `callsign_active` (`callsign`,`active`)
) ENGINE=InnoDB AUTO_INCREMENT=631312 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `aprs_duration`
--

DROP TABLE IF EXISTS `aprs_duration`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `aprs_duration` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `name` varchar(64) NOT NULL DEFAULT '',
  `amount` varchar(64) NOT NULL DEFAULT '',
  `short_name` varchar(64) NOT NULL,
  `seconds` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`),
  UNIQUE KEY `name` (`name`)
) ENGINE=InnoDB AUTO_INCREMENT=25 DEFAULT CHARSET=latin1 MAX_ROWS=1000000000;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `aprs_icons`
--

DROP TABLE IF EXISTS `aprs_icons`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `aprs_icons` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `symbol_table` char(1) CHARACTER SET utf8 COLLATE utf8_bin NOT NULL DEFAULT '/',
  `icon` char(1) CHARACTER SET utf8 COLLATE utf8_bin NOT NULL DEFAULT '',
  `gpsxyz` char(2) CHARACTER SET utf8 NOT NULL DEFAULT '',
  `index_no` int(11) NOT NULL DEFAULT '0',
  `name` varchar(64) CHARACTER SET utf8 NOT NULL DEFAULT '',
  `image` varchar(64) CHARACTER SET utf8 NOT NULL DEFAULT '',
  `icon_path` enum('primary','secondary','compass') CHARACTER SET utf8 DEFAULT 'primary',
  `direction` enum('Y','N') CHARACTER SET utf8 NOT NULL DEFAULT 'N',
  `google_earth_scale` float(3,2) DEFAULT '1.00',
  PRIMARY KEY (`id`),
  KEY `icon` (`icon`),
  KEY `symbol_table_code_idx` (`symbol_table`,`icon`)
) ENGINE=InnoDB AUTO_INCREMENT=209 DEFAULT CHARSET=latin1 MAX_ROWS=1000000000;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `aprs_limits`
--

DROP TABLE IF EXISTS `aprs_limits`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `aprs_limits` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `name` varchar(64) NOT NULL DEFAULT '',
  `amount` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`),
  UNIQUE KEY `name` (`name`)
) ENGINE=InnoDB AUTO_INCREMENT=24 DEFAULT CHARSET=latin1 MAX_ROWS=1000000000;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `aprs_searches`
--

DROP TABLE IF EXISTS `aprs_searches`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `aprs_searches` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `name` varchar(64) NOT NULL DEFAULT '',
  `type` varchar(64) DEFAULT 'map',
  `ip` varchar(64) NOT NULL DEFAULT '',
  `browser` varchar(64) NOT NULL DEFAULT '',
  `search_date` datetime NOT NULL DEFAULT '0000-00-00 00:00:00',
  `create_ts` int(11) NOT NULL DEFAULT '0',
  `callsign` varchar(64) NOT NULL DEFAULT '',
  `count` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`),
  UNIQUE KEY `name` (`name`),
  KEY `count` (`count`),
  KEY `search_date` (`search_date`)
) ENGINE=InnoDB AUTO_INCREMENT=516 DEFAULT CHARSET=latin1 MAX_ROWS=1000000000;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `aprsmail_filter`
--

DROP TABLE IF EXISTS `aprsmail_filter`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `aprsmail_filter` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `user_id` int(11) NOT NULL DEFAULT '0',
  `folder_id` int(11) NOT NULL DEFAULT '0',
  `priority` int(11) NOT NULL DEFAULT '0',
  `header_id` int(11) NOT NULL DEFAULT '0',
  `string` varchar(64) NOT NULL DEFAULT '',
  `action` enum('accept','accept_read','discard','reject','accept_rf') NOT NULL DEFAULT 'accept',
  `hits` int(11) NOT NULL DEFAULT '0',
  `create_ts` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`),
  KEY `user_id` (`user_id`),
  KEY `user_id_folder_id` (`user_id`,`folder_id`)
) ENGINE=InnoDB AUTO_INCREMENT=125 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `aprsmail_header`
--

DROP TABLE IF EXISTS `aprsmail_header`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `aprsmail_header` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `name` varchar(64) NOT NULL DEFAULT '',
  `match_string` varchar(64) NOT NULL DEFAULT '',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=10 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `aprsmail_log`
--

DROP TABLE IF EXISTS `aprsmail_log`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `aprsmail_log` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `user_id` int(11) NOT NULL DEFAULT '0',
  `reference_id` int(11) NOT NULL DEFAULT '0',
  `message` longtext,
  `create_ts` int(11) NOT NULL DEFAULT '0',
  `type` enum('folder','contacts','filters','draft','notes') NOT NULL DEFAULT 'folder',
  PRIMARY KEY (`id`),
  KEY `user_id_type` (`user_id`,`type`),
  KEY `user_id_reference_type` (`user_id`,`reference_id`,`type`)
) ENGINE=InnoDB AUTO_INCREMENT=65 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `aprsmail_mail`
--

DROP TABLE IF EXISTS `aprsmail_mail`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `aprsmail_mail` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `user_id` int(11) NOT NULL DEFAULT '0',
  `contact_id` int(11) NOT NULL DEFAULT '0',
  `sender` varchar(64) NOT NULL DEFAULT '',
  `callsign` varchar(10) DEFAULT NULL,
  `original` longtext NOT NULL,
  `mail_date` varchar(64) NOT NULL DEFAULT '',
  `mail_from` varchar(64) NOT NULL DEFAULT '',
  `mail_to` longtext,
  `mail_cc` longtext,
  `mail_bcc` longtext,
  `mail_subject` varchar(255) NOT NULL DEFAULT '',
  `mail_content` longtext,
  `errmsg` longtext NOT NULL,
  `create_ts` int(11) DEFAULT '0',
  `read_ts` int(11) NOT NULL DEFAULT '0',
  `folder` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`),
  KEY `folder_read_ts` (`folder`,`read_ts`),
  KEY `user_id` (`user_id`),
  KEY `user_id_read_ts` (`user_id`,`read_ts`),
  KEY `user_id_folder` (`user_id`,`folder`),
  KEY `user_id_mail_id` (`user_id`,`id`),
  KEY `received` (`create_ts`),
  KEY `read_time` (`read_ts`)
) ENGINE=InnoDB AUTO_INCREMENT=2757 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `aprsmail_options`
--

DROP TABLE IF EXISTS `aprsmail_options`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `aprsmail_options` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `user_id` int(11) NOT NULL DEFAULT '0',
  `name` varchar(64) NOT NULL DEFAULT '',
  `value` blob NOT NULL,
  `create_ts` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `cache_24hr`
--

DROP TABLE IF EXISTS `cache_24hr`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `cache_24hr` (
  `packet_id` char(32) NOT NULL,
  `callsign_id` int(11) NOT NULL,
  `path_id` int(11) NOT NULL,
  `status_id` int(11) NOT NULL DEFAULT '0',
  `packet_date` timestamp NOT NULL DEFAULT '0000-00-00 00:00:00',
  `time_of_fix` int(10) unsigned DEFAULT NULL,
  `latitude` double(8,6) DEFAULT NULL,
  `longitude` double(9,6) DEFAULT NULL,
  `course` smallint(5) unsigned DEFAULT NULL,
  `speed` smallint(3) unsigned DEFAULT NULL,
  `altitude` mediumint(5) DEFAULT NULL,
  `symbol_table` char(1) NOT NULL,
  `symbol_code` char(1) NOT NULL,
  `create_ts` int(14) DEFAULT '0',
  KEY `create_ts` (`create_ts`),
  KEY `source_ts` (`create_ts`),
  KEY `callsign_id` (`callsign_id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1
/*!50100 PARTITION BY RANGE ( UNIX_TIMESTAMP(packet_date))
(PARTITION 2017_10_24 VALUES LESS THAN (1508824800) ENGINE = InnoDB,
 PARTITION 2017_10_25 VALUES LESS THAN (1508911200) ENGINE = InnoDB,
 PARTITION 2017_10_26 VALUES LESS THAN (1508997600) ENGINE = InnoDB,
 PARTITION 2017_10_27 VALUES LESS THAN (1509084000) ENGINE = InnoDB,
 PARTITION 2017_10_28 VALUES LESS THAN (1509170400) ENGINE = InnoDB,
 PARTITION 2017_10_29 VALUES LESS THAN (1509256800) ENGINE = InnoDB,
 PARTITION 2017_10_30 VALUES LESS THAN (1509343200) ENGINE = InnoDB,
 PARTITION 2017_10_31 VALUES LESS THAN (1509429600) ENGINE = InnoDB,
 PARTITION 2017_11_01 VALUES LESS THAN (1509516000) ENGINE = InnoDB,
 PARTITION 2017_11_02 VALUES LESS THAN (1509602400) ENGINE = InnoDB,
 PARTITION 2017_11_03 VALUES LESS THAN (1509688800) ENGINE = InnoDB,
 PARTITION 2017_11_04 VALUES LESS THAN (1509775200) ENGINE = InnoDB) */;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `callsign`
--

DROP TABLE IF EXISTS `callsign`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `callsign` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `source` varchar(9) NOT NULL DEFAULT '',
  `created` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `create_ts` int(11) NOT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `source` (`source`)
) ENGINE=InnoDB AUTO_INCREMENT=819209 DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `cluster_lastposition`
--

DROP TABLE IF EXISTS `cluster_lastposition`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `cluster_lastposition` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `number` int(11) NOT NULL DEFAULT '0',
  `latitude1` decimal(11,6) DEFAULT '0.000000',
  `longitude1` decimal(11,6) DEFAULT '0.000000',
  `latitude2` double(11,6) NOT NULL DEFAULT '0.000000',
  `longitude2` double(11,6) NOT NULL DEFAULT '0.000000',
  `grid` varchar(64) NOT NULL,
  `type` enum('S','L','C') DEFAULT 'S',
  `create_ts` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`),
  KEY `latitude1` (`latitude1`,`longitude1`,`latitude2`,`longitude2`),
  KEY `type` (`type`)
) ENGINE=InnoDB AUTO_INCREMENT=28536 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `create_message`
--

DROP TABLE IF EXISTS `create_message`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `create_message` (
  `id` int(32) unsigned NOT NULL AUTO_INCREMENT,
  `source` varchar(9) NOT NULL,
  `target` varchar(9) NOT NULL DEFAULT '',
  `message` varchar(512) NOT NULL,
  `local` enum('Y','N') DEFAULT 'N',
  `ack` enum('Y','N') NOT NULL DEFAULT 'N',
  `msgnum` varchar(5) NOT NULL,
  `msgack` char(2) NOT NULL,
  `decay_id` char(32) NOT NULL,
  `error` enum('Y','N') NOT NULL DEFAULT 'N',
  `ack_ts` int(11) NOT NULL DEFAULT '0',
  `broadcast_ts` int(11) NOT NULL DEFAULT '0',
  `create_ts` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`),
  KEY `destination` (`message`(255)),
  KEY `source` (`source`),
  KEY `broadcast_error_idx` (`broadcast_ts`,`error`)
) ENGINE=InnoDB AUTO_INCREMENT=78720 DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `create_object`
--

DROP TABLE IF EXISTS `create_object`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `create_object` (
  `id` int(32) unsigned NOT NULL AUTO_INCREMENT,
  `name` varchar(9) NOT NULL DEFAULT '',
  `source` varchar(9) NOT NULL DEFAULT '',
  `destination` varchar(9) NOT NULL DEFAULT '',
  `latitude` double(8,6) NOT NULL DEFAULT '0.000000',
  `longitude` double(9,6) NOT NULL DEFAULT '0.000000',
  `course` smallint(5) unsigned DEFAULT NULL,
  `speed` smallint(5) unsigned DEFAULT NULL,
  `altitude` mediumint(9) DEFAULT NULL,
  `status` tinytext,
  `symbol_table` char(1) DEFAULT NULL,
  `symbol_code` char(1) DEFAULT NULL,
  `type` enum('P','O','I') NOT NULL DEFAULT 'P',
  `kill` enum('Y','N') NOT NULL DEFAULT 'N',
  `beacon` int(11) NOT NULL DEFAULT '0',
  `decay_id` char(32) NOT NULL,
  `error` enum('Y','N') NOT NULL DEFAULT 'N',
  `local` enum('Y','N') NOT NULL DEFAULT 'N',
  `broadcast_ts` int(11) NOT NULL DEFAULT '0',
  `expire_ts` int(11) NOT NULL DEFAULT '0',
  `create_ts` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`name`),
  UNIQUE KEY `id` (`id`),
  KEY `latitude` (`latitude`),
  KEY `longitude` (`longitude`),
  KEY `symbol_table` (`symbol_table`,`symbol_code`),
  KEY `destination` (`destination`)
) ENGINE=InnoDB AUTO_INCREMENT=254492 DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `create_position`
--

DROP TABLE IF EXISTS `create_position`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `create_position` (
  `id` int(32) unsigned NOT NULL AUTO_INCREMENT,
  `source` varchar(9) NOT NULL DEFAULT '',
  `destination` varchar(9) NOT NULL DEFAULT '',
  `latitude` double(8,6) NOT NULL DEFAULT '0.000000',
  `longitude` double(9,6) NOT NULL DEFAULT '0.000000',
  `course` smallint(5) unsigned DEFAULT NULL,
  `speed` smallint(5) unsigned DEFAULT NULL,
  `altitude` mediumint(9) DEFAULT NULL,
  `status` tinytext,
  `symbol_table` char(1) DEFAULT NULL,
  `symbol_code` char(1) DEFAULT NULL,
  `error` enum('Y','N') NOT NULL DEFAULT 'N',
  `local` enum('Y','N') NOT NULL DEFAULT 'N',
  `broadcast_ts` int(11) NOT NULL DEFAULT '0',
  `create_ts` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`source`),
  UNIQUE KEY `id` (`id`),
  KEY `latitude` (`latitude`),
  KEY `longitude` (`longitude`),
  KEY `symbol_table` (`symbol_table`,`symbol_code`),
  KEY `destination` (`destination`),
  KEY `broadcast_error_idx` (`broadcast_ts`,`error`)
) ENGINE=InnoDB AUTO_INCREMENT=10232856 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `daemon_error`
--

DROP TABLE IF EXISTS `daemon_error`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `daemon_error` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `message` longtext NOT NULL,
  `email_ts` int(11) NOT NULL DEFAULT '0',
  `create_ts` int(10) unsigned DEFAULT '0',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=205 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `daemon_status`
--

DROP TABLE IF EXISTS `daemon_status`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `daemon_status` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `name` varchar(64) DEFAULT NULL,
  `comment` varchar(64) NOT NULL,
  `count` int(10) unsigned DEFAULT '0',
  `hits` int(11) NOT NULL DEFAULT '0',
  `time` float(6,2) NOT NULL DEFAULT '0.00',
  `create_ts` int(10) unsigned DEFAULT '0',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=75996 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `destination`
--

DROP TABLE IF EXISTS `destination`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `destination` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `name` varchar(9) CHARACTER SET utf8 NOT NULL,
  `created_at` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `digis`
--

DROP TABLE IF EXISTS `digis`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `digis` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `name` varchar(9) NOT NULL DEFAULT '',
  `created_at` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`id`),
  UNIQUE KEY `name_uniq` (`name`),
  KEY `created_at` (`created_at`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `email_shortcuts`
--

DROP TABLE IF EXISTS `email_shortcuts`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `email_shortcuts` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `callsign` varchar(9) DEFAULT NULL,
  `alias` varchar(64) NOT NULL,
  `email` varchar(64) NOT NULL,
  `create_ts` int(10) unsigned DEFAULT '0',
  PRIMARY KEY (`id`),
  UNIQUE KEY `callsign` (`callsign`,`email`)
) ENGINE=InnoDB AUTO_INCREMENT=60 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `geonames`
--

DROP TABLE IF EXISTS `geonames`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `geonames` (
  `id` int(11) DEFAULT NULL,
  `name` varchar(200) DEFAULT NULL,
  `asciiname` varchar(200) DEFAULT NULL,
  `alternatenames` varchar(4000) DEFAULT NULL,
  `latitude` float(8,6) DEFAULT NULL,
  `longitude` float(9,6) DEFAULT NULL,
  `feature_class` char(1) DEFAULT NULL,
  `feature_code` varchar(10) DEFAULT NULL,
  `country_code` char(2) DEFAULT NULL,
  `cc2` varchar(60) DEFAULT NULL,
  `admin1_code` varchar(20) DEFAULT NULL,
  `admin2_code` varchar(20) DEFAULT NULL,
  `admin3_code` varchar(20) DEFAULT NULL,
  `admin4_code` varchar(20) DEFAULT NULL,
  `population` int(11) DEFAULT NULL,
  `elevation` int(11) DEFAULT NULL,
  `gtopo30` int(11) DEFAULT NULL,
  `timezone` varchar(128) DEFAULT NULL,
  `modification_date` date DEFAULT '0000-00-00',
  KEY `name` (`name`),
  KEY `asciiname` (`asciiname`),
  KEY `latitude` (`latitude`,`longitude`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `geonames_admin1_code`
--

DROP TABLE IF EXISTS `geonames_admin1_code`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `geonames_admin1_code` (
  `id` int(11) NOT NULL,
  `code` char(20) DEFAULT NULL,
  `name` varchar(200) DEFAULT NULL,
  `asciiname` varchar(200) DEFAULT NULL,
  KEY `code` (`code`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `geonames_admin2_code`
--

DROP TABLE IF EXISTS `geonames_admin2_code`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `geonames_admin2_code` (
  `id` int(11) NOT NULL,
  `code` char(20) DEFAULT NULL,
  `name` varchar(200) DEFAULT NULL,
  `asciiname` varchar(200) DEFAULT NULL,
  KEY `code` (`code`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `geonames_country`
--

DROP TABLE IF EXISTS `geonames_country`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `geonames_country` (
  `id` int(11) NOT NULL,
  `iso_alpha2` char(2) DEFAULT NULL,
  `iso_alpha3` char(3) DEFAULT NULL,
  `iso_numeric` int(11) DEFAULT NULL,
  `fips_code` char(2) DEFAULT NULL,
  `name` varchar(200) DEFAULT NULL,
  `capital` varchar(200) DEFAULT NULL,
  `area` int(11) DEFAULT NULL,
  `population` int(11) DEFAULT NULL,
  `continent` char(2) DEFAULT NULL,
  `languages` varchar(200) DEFAULT NULL,
  `currency` varchar(20) DEFAULT NULL,
  KEY `iso_alpha2` (`iso_alpha2`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `geonames_zip`
--

DROP TABLE IF EXISTS `geonames_zip`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `geonames_zip` (
  `country_code` char(2) NOT NULL,
  `postal_code` varchar(10) NOT NULL,
  `place_name` varchar(180) DEFAULT NULL,
  `admin_name1` varchar(100) DEFAULT NULL,
  `admin_code1` varchar(20) DEFAULT NULL,
  `admin_name2` varchar(100) DEFAULT NULL,
  `admin_code2` varchar(20) DEFAULT NULL,
  `admin_name3` varchar(100) DEFAULT NULL,
  `latitude` float(8,6) DEFAULT NULL,
  `longitude` float(9,6) DEFAULT NULL,
  `accuracy` int(11) DEFAULT NULL,
  KEY `postal_code` (`postal_code`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `igated`
--

DROP TABLE IF EXISTS `igated`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `igated` (
  `packet_id` char(32) NOT NULL,
  `callsign_id` int(11) NOT NULL,
  `igate` varchar(9) NOT NULL DEFAULT '',
  `source` varchar(9) NOT NULL DEFAULT '',
  `latitude` double(8,6) NOT NULL DEFAULT '0.000000',
  `longitude` double(9,6) NOT NULL DEFAULT '0.000000',
  `create_ts` int(11) NOT NULL DEFAULT '0',
  KEY `latitude` (`latitude`),
  KEY `longitude` (`longitude`),
  KEY `packet_id` (`packet_id`),
  KEY `igate` (`igate`,`source`),
  KEY `callsign_id` (`callsign_id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `irlp`
--

DROP TABLE IF EXISTS `irlp`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `irlp` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `node` int(11) NOT NULL DEFAULT '0',
  `callsign` varchar(64) NOT NULL DEFAULT '',
  `city` varchar(64) NOT NULL DEFAULT '',
  `state` varchar(64) NOT NULL DEFAULT '',
  `country` varchar(64) NOT NULL DEFAULT '',
  `status` varchar(64) NOT NULL DEFAULT '',
  `record` varchar(64) NOT NULL DEFAULT '',
  `install_date` date NOT NULL DEFAULT '0000-00-00',
  `latitude` double(8,6) NOT NULL DEFAULT '0.000000',
  `longitude` double(9,6) NOT NULL DEFAULT '0.000000',
  `lastupdate_ts` int(11) NOT NULL DEFAULT '0',
  `frequency` double(9,6) NOT NULL DEFAULT '0.000000',
  `offset` double(9,6) NOT NULL DEFAULT '0.000000',
  `pl` double(9,6) NOT NULL DEFAULT '0.000000',
  `owner` varchar(64) NOT NULL DEFAULT '',
  `url` varchar(64) NOT NULL DEFAULT '',
  `laststatus_ts` int(11) NOT NULL DEFAULT '0',
  `create_ts` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`),
  KEY `node` (`node`),
  KEY `status` (`status`),
  KEY `status_order` (`status`,`country`,`state`,`city`),
  KEY `ordered` (`country`,`state`,`city`),
  KEY `longitude` (`longitude`,`latitude`)
) ENGINE=InnoDB AUTO_INCREMENT=40492746 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `last_bulletin`
--

DROP TABLE IF EXISTS `last_bulletin`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `last_bulletin` (
  `packet_id` bigint(20) NOT NULL,
  `callsign_id` int(11) NOT NULL,
  `addressee` varchar(10) NOT NULL DEFAULT '',
  `text` varchar(73) DEFAULT NULL,
  `id` varchar(10) DEFAULT NULL,
  `create_ts` int(14) NOT NULL DEFAULT '0',
  PRIMARY KEY (`callsign_id`),
  KEY `packet_id` (`packet_id`),
  KEY `create_ts` (`create_ts`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `last_dfr`
--

DROP TABLE IF EXISTS `last_dfr`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `last_dfr` (
  `packet_id` bigint(20) NOT NULL,
  `callsign_id` int(11) NOT NULL,
  `name_id` int(11) NOT NULL,
  `name` varchar(9) NOT NULL,
  `bearing` int(11) DEFAULT NULL,
  `hits` int(11) NOT NULL,
  `range` double(9,2) NOT NULL,
  `quality` int(11) NOT NULL,
  `create_ts` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`callsign_id`,`name`),
  KEY `packet_id` (`packet_id`),
  KEY `create_ts` (`create_ts`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `last_dfs`
--

DROP TABLE IF EXISTS `last_dfs`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `last_dfs` (
  `packet_id` bigint(20) NOT NULL,
  `callsign_id` int(11) NOT NULL,
  `name_id` int(11) NOT NULL,
  `packet_date` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  `power` double(10,2) DEFAULT NULL,
  `haat` double(10,2) DEFAULT NULL,
  `gain` double(10,2) DEFAULT NULL,
  `range` double(10,2) DEFAULT NULL,
  `direction` int(11) DEFAULT NULL,
  `create_ts` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`callsign_id`,`name_id`),
  KEY `packet_id` (`packet_id`),
  KEY `create_ts` (`create_ts`),
  KEY `packet_date` (`packet_date`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `last_frequency`
--

DROP TABLE IF EXISTS `last_frequency`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `last_frequency` (
  `packet_id` bigint(20) NOT NULL,
  `callsign_id` int(11) NOT NULL,
  `name` varchar(9) NOT NULL DEFAULT '',
  `frequency` varchar(11) NOT NULL,
  `range` double(9,2) DEFAULT NULL,
  `range_east` double(9,2) DEFAULT NULL,
  `tone` varchar(6) DEFAULT NULL,
  `afrs_type` enum('pl','ctcss','dcs','burst') DEFAULT NULL,
  `receive` varchar(7) DEFAULT NULL,
  `alternate` varchar(7) DEFAULT NULL,
  `type` enum('P','O','I') NOT NULL DEFAULT 'P',
  `create_ts` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`callsign_id`),
  KEY `packet_id` (`packet_id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `last_message`
--

DROP TABLE IF EXISTS `last_message`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `last_message` (
  `packet_id` bigint(20) NOT NULL,
  `callsign_id` int(11) NOT NULL,
  `callsign_to_id` int(11) NOT NULL,
  `addressee` varchar(10) CHARACTER SET utf8 DEFAULT NULL,
  `text` varchar(73) CHARACTER SET utf8 DEFAULT NULL,
  `id` varchar(10) CHARACTER SET utf8 DEFAULT NULL,
  `create_ts` int(14) NOT NULL DEFAULT '0',
  PRIMARY KEY (`callsign_id`),
  KEY `addressee` (`addressee`),
  KEY `packet_id` (`packet_id`),
  KEY `create_ts` (`create_ts`),
  KEY `callsign_to_id` (`callsign_to_id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `last_object`
--

DROP TABLE IF EXISTS `last_object`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `last_object` (
  `packet_id` bigint(20) NOT NULL,
  `callsign_id` int(11) NOT NULL,
  `name` varchar(9) NOT NULL,
  `destination` varchar(9) NOT NULL DEFAULT '',
  `latitude` double(8,6) NOT NULL DEFAULT '0.000000',
  `longitude` double(9,6) NOT NULL DEFAULT '0.000000',
  `course` smallint(5) unsigned DEFAULT NULL,
  `speed` smallint(5) unsigned DEFAULT NULL,
  `altitude` mediumint(9) DEFAULT NULL,
  `status` tinytext,
  `symbol_table` char(1) DEFAULT NULL,
  `symbol_code` char(1) DEFAULT NULL,
  `create_ts` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`callsign_id`),
  KEY `latitude` (`latitude`),
  KEY `longitude` (`longitude`),
  KEY `packet_id` (`packet_id`),
  KEY `speed` (`speed`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `last_phg`
--

DROP TABLE IF EXISTS `last_phg`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `last_phg` (
  `packet_id` bigint(20) NOT NULL,
  `callsign_id` int(11) NOT NULL,
  `name_id` int(11) NOT NULL,
  `packet_date` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  `power` double(10,2) DEFAULT NULL,
  `haat` double(10,2) DEFAULT NULL,
  `gain` double(10,2) DEFAULT NULL,
  `range` double(10,2) DEFAULT NULL,
  `direction` int(11) DEFAULT NULL,
  `beacon` int(11) DEFAULT NULL,
  `create_ts` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`callsign_id`,`name_id`),
  KEY `packet_id` (`packet_id`),
  KEY `create_ts` (`create_ts`),
  KEY `callsign_id` (`callsign_id`),
  KEY `packet_date` (`packet_date`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `last_position`
--

DROP TABLE IF EXISTS `last_position`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `last_position` (
  `packet_id` bigint(20) NOT NULL,
  `callsign_id` int(11) NOT NULL,
  `name_id` int(11) NOT NULL,
  `packet_date` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  `name` varchar(9) CHARACTER SET latin1 NOT NULL DEFAULT '',
  `latitude` double(8,6) NOT NULL DEFAULT '0.000000',
  `longitude` double(9,6) NOT NULL DEFAULT '0.000000',
  PRIMARY KEY (`name`,`callsign_id`),
  KEY `latitude` (`latitude`),
  KEY `longitude` (`longitude`),
  KEY `packet_id` (`packet_id`),
  KEY `callsign_id` (`callsign_id`),
  KEY `lat_lng_create_idx` (`latitude`,`longitude`),
  KEY `packet_date` (`packet_date`),
  KEY `lat_lng_pdate_idx` (`latitude`,`longitude`,`packet_date`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `last_position_meta`
--

DROP TABLE IF EXISTS `last_position_meta`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `last_position_meta` (
  `packet_id` bigint(20) NOT NULL,
  `callsign_id` int(11) NOT NULL,
  `name_id` int(11) NOT NULL,
  `dest_id` int(11) NOT NULL DEFAULT '0',
  `path_id` int(11) NOT NULL,
  `course` smallint(5) unsigned DEFAULT NULL,
  `speed` smallint(5) unsigned DEFAULT NULL,
  `altitude` mediumint(9) DEFAULT NULL,
  `status_id` int(11) NOT NULL DEFAULT '0',
  `symbol_table` char(1) DEFAULT NULL,
  `symbol_code` char(1) DEFAULT NULL,
  `overlay` varchar(3) DEFAULT NULL,
  `range` double(10,2) DEFAULT NULL,
  `type` enum('P','O','I') NOT NULL DEFAULT 'P',
  `weather` enum('Y','N') NOT NULL DEFAULT 'N',
  `telemetry` enum('Y','N') NOT NULL DEFAULT 'N',
  `igate` enum('Y','N') NOT NULL DEFAULT 'N',
  `position_type_id` tinyint(11) NOT NULL,
  `mbits` varchar(3) DEFAULT NULL,
  `packet_date` timestamp NOT NULL DEFAULT '0000-00-00 00:00:00',
  `created_at` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`callsign_id`,`name_id`),
  KEY `packet_id` (`packet_id`),
  KEY `symbol_table` (`symbol_table`,`symbol_code`),
  KEY `destination` (`dest_id`),
  KEY `create_ts` (`created_at`),
  KEY `weather` (`weather`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `last_telemetry`
--

DROP TABLE IF EXISTS `last_telemetry`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `last_telemetry` (
  `packet_id` bigint(20) NOT NULL,
  `callsign_id` int(11) NOT NULL,
  `packet_date` datetime NOT NULL DEFAULT '0000-00-00 00:00:00',
  `sequence` int(3) NOT NULL DEFAULT '0',
  `analog_0` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `analog_1` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `analog_2` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `analog_3` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `analog_4` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `digital` char(8) NOT NULL DEFAULT '000000',
  `create_ts` int(14) NOT NULL DEFAULT '0',
  PRIMARY KEY (`callsign_id`),
  KEY `create_ts` (`create_ts`),
  KEY `sequence` (`sequence`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `lastposition`
--

DROP TABLE IF EXISTS `lastposition`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `lastposition` (
  `packet_id` bigint(20) unsigned DEFAULT NULL,
  `callsign_id` int(11) NOT NULL,
  `name_id` int(11) NOT NULL,
  `packet_date` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  `name` varchar(9) CHARACTER SET latin1 NOT NULL DEFAULT '',
  `destination` varchar(9) CHARACTER SET latin1 NOT NULL DEFAULT '',
  `path_id` int(11) NOT NULL,
  `latitude` double(8,6) NOT NULL DEFAULT '0.000000',
  `longitude` double(9,6) NOT NULL DEFAULT '0.000000',
  `course` smallint(5) unsigned DEFAULT NULL,
  `speed` smallint(5) unsigned DEFAULT NULL,
  `altitude` mediumint(9) DEFAULT NULL,
  `status_id` int(11) NOT NULL DEFAULT '0',
  `symbol_table` char(1) CHARACTER SET latin1 DEFAULT NULL,
  `symbol_code` char(1) CHARACTER SET latin1 DEFAULT NULL,
  `overlay` varchar(3) CHARACTER SET latin1 DEFAULT NULL,
  `phg_power` double(10,2) DEFAULT NULL,
  `phg_haat` double(10,2) DEFAULT NULL,
  `phg_gain` double(10,2) DEFAULT NULL,
  `phg_range` double(10,2) DEFAULT NULL,
  `phg_direction` int(11) DEFAULT NULL,
  `phg_beacon` int(11) DEFAULT NULL,
  `dfs_power` double(10,2) DEFAULT NULL,
  `dfs_haat` double(10,2) DEFAULT NULL,
  `dfs_gain` double(10,2) DEFAULT NULL,
  `dfs_range` double(10,2) DEFAULT NULL,
  `dfs_direction` int(11) DEFAULT NULL,
  `range` double(10,2) DEFAULT NULL,
  `type` enum('P','O','I') CHARACTER SET latin1 NOT NULL DEFAULT 'P',
  `weather` enum('Y','N') CHARACTER SET latin1 NOT NULL DEFAULT 'N',
  `telemetry` enum('Y','N') CHARACTER SET latin1 NOT NULL DEFAULT 'N',
  `igate` enum('Y','N') CHARACTER SET latin1 NOT NULL DEFAULT 'N',
  `postype` enum('UNCOMPRESSED','COMPRESSED','NMEA','MIC_E') CHARACTER SET latin1 NOT NULL DEFAULT 'UNCOMPRESSED',
  `mbits` varchar(3) CHARACTER SET latin1 DEFAULT NULL,
  `create_ts` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`name`,`callsign_id`),
  KEY `latitude` (`latitude`),
  KEY `longitude` (`longitude`),
  KEY `packet_id` (`packet_id`),
  KEY `destination` (`destination`),
  KEY `create_ts` (`create_ts`),
  KEY `callsign_id` (`callsign_id`),
  KEY `lat_lng_create_idx` (`latitude`,`longitude`,`create_ts`),
  KEY `packet_date` (`packet_date`),
  KEY `lat_lng_pdate_idx` (`latitude`,`longitude`,`packet_date`),
  KEY `symbol_table_code_idx` (`symbol_table`,`symbol_code`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `lastraw`
--

DROP TABLE IF EXISTS `lastraw`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `lastraw` (
  `packet_id` bigint(20) NOT NULL,
  `callsign_id` int(11) NOT NULL,
  `ip_source` varchar(255) CHARACTER SET latin1 DEFAULT NULL,
  `destination` varchar(9) CHARACTER SET latin1 NOT NULL DEFAULT '',
  `digipeater_0` varchar(9) CHARACTER SET latin1 DEFAULT NULL,
  `digipeater_1` varchar(9) CHARACTER SET latin1 DEFAULT NULL,
  `digipeater_2` varchar(9) CHARACTER SET latin1 DEFAULT NULL,
  `digipeater_3` varchar(9) CHARACTER SET latin1 DEFAULT NULL,
  `digipeater_4` varchar(9) CHARACTER SET latin1 DEFAULT NULL,
  `digipeater_5` varchar(9) CHARACTER SET latin1 DEFAULT NULL,
  `digipeater_6` varchar(9) CHARACTER SET latin1 DEFAULT NULL,
  `digipeater_7` varchar(9) CHARACTER SET latin1 DEFAULT NULL,
  `information` longtext CHARACTER SET latin1,
  `hostid` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `create_ts` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`callsign_id`),
  KEY `create_ts` (`create_ts`),
  KEY `digipeater_0` (`digipeater_0`,`digipeater_1`,`digipeater_2`,`digipeater_3`,`digipeater_4`,`digipeater_5`,`digipeater_6`,`digipeater_7`),
  KEY `packet_id` (`packet_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `laststatus`
--

DROP TABLE IF EXISTS `laststatus`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `laststatus` (
  `packet_id` bigint(20) NOT NULL,
  `callsign_id` int(11) NOT NULL,
  `packet_date` datetime NOT NULL DEFAULT '0000-00-00 00:00:00',
  `comment` tinytext,
  `power` tinyint(4) DEFAULT NULL,
  `height` smallint(5) unsigned DEFAULT NULL,
  `gain` tinyint(4) DEFAULT NULL,
  `directivity` tinyint(4) DEFAULT NULL,
  `rate` tinyint(4) DEFAULT NULL,
  `symbol_table` char(1) DEFAULT NULL,
  `symbol_code` char(1) DEFAULT NULL,
  `create_ts` int(14) NOT NULL DEFAULT '0',
  PRIMARY KEY (`callsign_id`),
  KEY `rate` (`rate`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `lastweather`
--

DROP TABLE IF EXISTS `lastweather`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `lastweather` (
  `packet_id` bigint(20) unsigned DEFAULT NULL,
  `callsign_id` int(11) NOT NULL,
  `packet_date` datetime NOT NULL DEFAULT '0000-00-00 00:00:00',
  `latitude` double(8,6) DEFAULT '0.000000',
  `longitude` double(9,6) DEFAULT '0.000000',
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
  PRIMARY KEY (`callsign_id`),
  KEY `packet_id` (`packet_id`),
  KEY `latitude` (`latitude`),
  KEY `longitude` (`longitude`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `line_colors`
--

DROP TABLE IF EXISTS `line_colors`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `line_colors` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `name` varchar(64) NOT NULL DEFAULT '',
  `color` varchar(64) NOT NULL DEFAULT '',
  `width` int(11) NOT NULL DEFAULT '4',
  `create_ts` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`),
  KEY `name` (`name`)
) ENGINE=InnoDB AUTO_INCREMENT=10 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `mem_lastposition`
--

DROP TABLE IF EXISTS `mem_lastposition`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `mem_lastposition` (
  `packet_id` bigint(20) NOT NULL,
  `callsign_id` int(11) NOT NULL,
  `name_id` int(11) NOT NULL,
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
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `mem_position`
--

DROP TABLE IF EXISTS `mem_position`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `mem_position` (
  `callsign_id` int(11) NOT NULL,
  `packet_date` timestamp NOT NULL DEFAULT '0000-00-00 00:00:00',
  `latitude` double(8,6) DEFAULT NULL,
  `longitude` double(9,6) DEFAULT NULL,
  KEY `callsign_id` (`callsign_id`),
  KEY `packet_date` (`packet_date`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8
/*!50100 PARTITION BY RANGE ( UNIX_TIMESTAMP(packet_date))
(PARTITION 2017_10_25 VALUES LESS THAN (1508911200) ENGINE = InnoDB,
 PARTITION 2017_10_26 VALUES LESS THAN (1508997600) ENGINE = InnoDB,
 PARTITION 2017_10_27 VALUES LESS THAN (1509084000) ENGINE = InnoDB,
 PARTITION 2017_10_28 VALUES LESS THAN (1509170400) ENGINE = InnoDB,
 PARTITION 2017_10_29 VALUES LESS THAN (1509256800) ENGINE = InnoDB,
 PARTITION 2017_10_30 VALUES LESS THAN (1509343200) ENGINE = InnoDB,
 PARTITION 2017_10_31 VALUES LESS THAN (1509429600) ENGINE = InnoDB,
 PARTITION 2017_11_01 VALUES LESS THAN (1509516000) ENGINE = InnoDB,
 PARTITION 2017_11_02 VALUES LESS THAN (1509602400) ENGINE = InnoDB,
 PARTITION 2017_11_03 VALUES LESS THAN (1509688800) ENGINE = InnoDB,
 PARTITION 2017_11_04 VALUES LESS THAN (1509775200) ENGINE = InnoDB) */;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `menu`
--

DROP TABLE IF EXISTS `menu`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `menu` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `user_id` int(11) NOT NULL DEFAULT '0',
  `title` varchar(128) NOT NULL DEFAULT '',
  `url` varchar(128) NOT NULL DEFAULT '',
  `body` longtext,
  `type` enum('normal','menu_right','ge') NOT NULL DEFAULT 'normal',
  `priority` int(11) NOT NULL DEFAULT '0',
  `click_count` int(11) NOT NULL DEFAULT '0',
  `create_ts` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=35 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `message`
--

DROP TABLE IF EXISTS `message`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `message` (
  `id` bigint(20) unsigned NOT NULL DEFAULT '0',
  `body` longtext,
  PRIMARY KEY (`id`),
  KEY `body_idx` (`body`(32))
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `messages`
--

DROP TABLE IF EXISTS `messages`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `messages` (
  `packet_id` bigint(20) NOT NULL,
  `callsign_id` int(11) NOT NULL,
  `callsign_to_id` int(11) NOT NULL,
  `source` varchar(9) NOT NULL DEFAULT '',
  `packet_date` timestamp NOT NULL DEFAULT '0000-00-00 00:00:00',
  `addressee` varchar(10) DEFAULT NULL,
  `text` longtext,
  `id` varchar(10) DEFAULT NULL,
  `pushed` enum('Y','N') NOT NULL DEFAULT 'N',
  `create_ts` int(14) NOT NULL DEFAULT '0',
  KEY `source` (`source`),
  KEY `addressee` (`addressee`),
  KEY `create_ts` (`create_ts`),
  KEY `callsign_id` (`callsign_id`),
  KEY `callsign_to_id` (`callsign_to_id`),
  KEY `callsign_to_id_idx` (`callsign_to_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `object_expires`
--

DROP TABLE IF EXISTS `object_expires`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `object_expires` (
  `packet_id` int(14) unsigned DEFAULT '0',
  `source` varchar(9) NOT NULL DEFAULT '',
  `create_ts` int(11) unsigned DEFAULT '0',
  KEY `posted` (`create_ts`),
  KEY `packet_id` (`packet_id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `object_name`
--

DROP TABLE IF EXISTS `object_name`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `object_name` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `name` varchar(9) NOT NULL DEFAULT '',
  `created` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `create_ts` int(11) NOT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `name_uniq` (`name`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `packet`
--

DROP TABLE IF EXISTS `packet`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `packet` (
  `id` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `callsign_id` int(11) NOT NULL,
  `packet_date` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `create_ts` int(11) NOT NULL,
  PRIMARY KEY (`id`,`packet_date`),
  KEY `callsign_id` (`callsign_id`)
) ENGINE=InnoDB AUTO_INCREMENT=9449945153 DEFAULT CHARSET=utf8
/*!50100 PARTITION BY RANGE ( UNIX_TIMESTAMP(packet_date))
(PARTITION 2017_10_24 VALUES LESS THAN (1508824800) ENGINE = InnoDB,
 PARTITION 2017_10_25 VALUES LESS THAN (1508911200) ENGINE = InnoDB,
 PARTITION 2017_10_26 VALUES LESS THAN (1508997600) ENGINE = InnoDB,
 PARTITION 2017_10_27 VALUES LESS THAN (1509084000) ENGINE = InnoDB,
 PARTITION 2017_10_28 VALUES LESS THAN (1509170400) ENGINE = InnoDB,
 PARTITION 2017_10_29 VALUES LESS THAN (1509256800) ENGINE = InnoDB,
 PARTITION 2017_10_30 VALUES LESS THAN (1509343200) ENGINE = InnoDB,
 PARTITION 2017_10_31 VALUES LESS THAN (1509429600) ENGINE = InnoDB,
 PARTITION 2017_11_01 VALUES LESS THAN (1509516000) ENGINE = InnoDB,
 PARTITION 2017_11_02 VALUES LESS THAN (1509602400) ENGINE = InnoDB,
 PARTITION 2017_11_03 VALUES LESS THAN (1509688800) ENGINE = InnoDB,
 PARTITION 2017_11_04 VALUES LESS THAN (1509775200) ENGINE = InnoDB) */;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `path`
--

DROP TABLE IF EXISTS `path`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `path` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `hash` char(32) NOT NULL,
  `body` varchar(128) NOT NULL,
  `packet_date` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `create_ts` int(11) NOT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `hash` (`hash`),
  KEY `callsign_id` (`body`),
  KEY `packet_date` (`packet_date`)
) ENGINE=InnoDB AUTO_INCREMENT=15281621 DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `position`
--

DROP TABLE IF EXISTS `position`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `position` (
  `packet_id` char(32) CHARACTER SET latin1 NOT NULL,
  `callsign_id` bigint(20) NOT NULL DEFAULT '0',
  `status_id` int(11) NOT NULL,
  `path_id` int(11) DEFAULT '0',
  `packet_date` datetime NOT NULL DEFAULT '0000-00-00 00:00:00',
  `time_of_fix` int(10) unsigned DEFAULT NULL,
  `latitude` double(8,6) DEFAULT NULL,
  `longitude` double(9,6) DEFAULT NULL,
  `course` smallint(5) unsigned DEFAULT NULL,
  `speed` smallint(3) unsigned DEFAULT NULL,
  `altitude` mediumint(5) DEFAULT NULL,
  `symbol_table` char(1) CHARACTER SET latin1 NOT NULL,
  `symbol_code` char(1) CHARACTER SET latin1 NOT NULL,
  `create_ts` int(11) DEFAULT NULL,
  KEY `create_ts` (`create_ts`),
  KEY `source` (`callsign_id`,`create_ts`),
  KEY `callsign_id` (`callsign_id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `position_type`
--

DROP TABLE IF EXISTS `position_type`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `position_type` (
  `id` tinyint(4) NOT NULL AUTO_INCREMENT,
  `name` varchar(64) NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=6 DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `raw`
--

DROP TABLE IF EXISTS `raw`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `raw` (
  `packet_id` int(11) NOT NULL,
  `callsign_id` int(11) NOT NULL,
  `packet_date` timestamp NOT NULL DEFAULT '0000-00-00 00:00:00',
  `information` longtext CHARACTER SET latin1,
  `create_ts` int(11) NOT NULL DEFAULT '0',
  KEY `create_ts` (`create_ts`),
  KEY `packet_date` (`packet_date`),
  KEY `callsign_id` (`callsign_id`),
  KEY `packet_id` (`packet_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8
/*!50100 PARTITION BY RANGE ( UNIX_TIMESTAMP(packet_date))
(PARTITION 2017_10_25 VALUES LESS THAN (1508911200) ENGINE = InnoDB,
 PARTITION 2017_10_26 VALUES LESS THAN (1508997600) ENGINE = InnoDB,
 PARTITION 2017_10_27 VALUES LESS THAN (1509084000) ENGINE = InnoDB,
 PARTITION 2017_10_28 VALUES LESS THAN (1509170400) ENGINE = InnoDB,
 PARTITION 2017_10_29 VALUES LESS THAN (1509256800) ENGINE = InnoDB,
 PARTITION 2017_10_30 VALUES LESS THAN (1509343200) ENGINE = InnoDB,
 PARTITION 2017_10_31 VALUES LESS THAN (1509429600) ENGINE = InnoDB,
 PARTITION 2017_11_01 VALUES LESS THAN (1509516000) ENGINE = InnoDB,
 PARTITION 2017_11_02 VALUES LESS THAN (1509602400) ENGINE = InnoDB,
 PARTITION 2017_11_03 VALUES LESS THAN (1509688800) ENGINE = InnoDB,
 PARTITION 2017_11_04 VALUES LESS THAN (1509775200) ENGINE = InnoDB) */;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `raw_errors`
--

DROP TABLE IF EXISTS `raw_errors`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `raw_errors` (
  `packet_id` char(32) NOT NULL,
  `callsign_id` int(11) NOT NULL,
  `packet_date` timestamp NOT NULL DEFAULT '0000-00-00 00:00:00',
  `message` longtext NOT NULL,
  `create_ts` int(10) unsigned DEFAULT '0',
  KEY `create_ts` (`create_ts`),
  KEY `raw_id` (`packet_id`),
  KEY `packet_date` (`packet_date`),
  KEY `callsign_id` (`callsign_id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `raw_meta`
--

DROP TABLE IF EXISTS `raw_meta`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `raw_meta` (
  `packet_id` char(32) CHARACTER SET latin1 NOT NULL,
  `callsign_id` int(11) NOT NULL,
  `dest_id` int(11) NOT NULL,
  `packet_time` int(10) unsigned NOT NULL DEFAULT '0',
  `packet_date` timestamp NOT NULL DEFAULT '0000-00-00 00:00:00',
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
  KEY `create_ts` (`create_ts`),
  KEY `packet_date` (`packet_date`),
  KEY `callsign_id` (`callsign_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8
/*!50100 PARTITION BY RANGE ( UNIX_TIMESTAMP(packet_date))
(PARTITION 2017_10_25 VALUES LESS THAN (1508911200) ENGINE = InnoDB,
 PARTITION 2017_10_26 VALUES LESS THAN (1508997600) ENGINE = InnoDB,
 PARTITION 2017_10_27 VALUES LESS THAN (1509084000) ENGINE = InnoDB,
 PARTITION 2017_10_28 VALUES LESS THAN (1509170400) ENGINE = InnoDB,
 PARTITION 2017_10_29 VALUES LESS THAN (1509256800) ENGINE = InnoDB,
 PARTITION 2017_10_30 VALUES LESS THAN (1509343200) ENGINE = InnoDB,
 PARTITION 2017_10_31 VALUES LESS THAN (1509429600) ENGINE = InnoDB,
 PARTITION 2017_11_01 VALUES LESS THAN (1509516000) ENGINE = InnoDB,
 PARTITION 2017_11_02 VALUES LESS THAN (1509602400) ENGINE = InnoDB,
 PARTITION 2017_11_03 VALUES LESS THAN (1509688800) ENGINE = InnoDB,
 PARTITION 2017_11_04 VALUES LESS THAN (1509775200) ENGINE = InnoDB) */;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `satellite_folders`
--

DROP TABLE IF EXISTS `satellite_folders`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `satellite_folders` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `color_id` int(11) NOT NULL DEFAULT '1',
  `name` varchar(64) NOT NULL DEFAULT '',
  `file_name` varchar(64) NOT NULL DEFAULT 'amateur.txt',
  `kmz_name` varchar(64) NOT NULL DEFAULT '',
  `duration` int(11) NOT NULL DEFAULT '3600',
  `step` int(11) NOT NULL DEFAULT '120',
  `visible` enum('1','0') NOT NULL DEFAULT '0',
  `open` enum('1','0') NOT NULL DEFAULT '0',
  `enabled` enum('Y','N') NOT NULL DEFAULT 'N',
  `create_ts` int(11) NOT NULL DEFAULT '0',
  `update_ts` int(11) NOT NULL DEFAULT '0',
  `write_ts` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`),
  KEY `name` (`name`)
) ENGINE=InnoDB AUTO_INCREMENT=25 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `satellite_lastposition`
--

DROP TABLE IF EXISTS `satellite_lastposition`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `satellite_lastposition` (
  `number` varchar(10) NOT NULL DEFAULT '0',
  `packet_date` datetime NOT NULL DEFAULT '0000-00-00 00:00:00',
  `name` varchar(64) NOT NULL DEFAULT '',
  `elevation` int(11) NOT NULL DEFAULT '0',
  `azimuth` int(11) NOT NULL DEFAULT '0',
  `ra` double(11,6) NOT NULL DEFAULT '0.000000',
  `orbit_phase` int(11) NOT NULL DEFAULT '0',
  `latitude` decimal(11,6) NOT NULL DEFAULT '0.000000',
  `longitude` decimal(11,6) NOT NULL DEFAULT '0.000000',
  `slant_range` int(11) NOT NULL DEFAULT '0',
  `altitude` double(16,6) DEFAULT '0.000000',
  `orbit_num` int(11) NOT NULL DEFAULT '0',
  `moon_phase` int(1) NOT NULL DEFAULT '0',
  `create_ts` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`number`),
  KEY `packet_date` (`packet_date`),
  KEY `name` (`name`),
  KEY `create_ts` (`create_ts`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `satellite_names`
--

DROP TABLE IF EXISTS `satellite_names`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `satellite_names` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `number` varchar(32) DEFAULT NULL,
  `name` varchar(64) NOT NULL DEFAULT '',
  `display` varchar(64) NOT NULL DEFAULT '',
  `url` longtext,
  `description` longtext NOT NULL,
  `image_name` varchar(64) NOT NULL DEFAULT 'SATELLITE',
  `image_path` varchar(64) NOT NULL DEFAULT 'satellite.png',
  `color` varchar(64) NOT NULL DEFAULT 'YELLOW',
  `label` varchar(64) NOT NULL DEFAULT '',
  `folder_id` int(11) DEFAULT '1',
  `compass` enum('Y','N') NOT NULL DEFAULT 'N',
  `create_ts` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`),
  KEY `name` (`name`),
  KEY `folder_id` (`folder_id`)
) ENGINE=InnoDB AUTO_INCREMENT=1072 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `satellite_next120positions`
--

DROP TABLE IF EXISTS `satellite_next120positions`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `satellite_next120positions` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `number` varchar(10) DEFAULT '0',
  `packet_date` datetime NOT NULL DEFAULT '0000-00-00 00:00:00',
  `name` varchar(64) NOT NULL DEFAULT '',
  `elevation` int(11) NOT NULL DEFAULT '0',
  `azimuth` int(11) NOT NULL DEFAULT '0',
  `ra` double(11,6) NOT NULL DEFAULT '0.000000',
  `orbit_phase` int(11) NOT NULL DEFAULT '0',
  `latitude` decimal(11,6) NOT NULL DEFAULT '0.000000',
  `longitude` decimal(11,6) NOT NULL DEFAULT '0.000000',
  `slant_range` int(11) NOT NULL DEFAULT '0',
  `altitude` double(16,6) DEFAULT '0.000000',
  `orbit_num` int(11) NOT NULL DEFAULT '0',
  `sequence` int(11) NOT NULL,
  `create_ts` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`),
  KEY `packet_date` (`packet_date`),
  KEY `name` (`name`),
  KEY `create_ts` (`create_ts`),
  KEY `number` (`number`,`create_ts`)
) ENGINE=InnoDB AUTO_INCREMENT=1628433059 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `satellite_position`
--

DROP TABLE IF EXISTS `satellite_position`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `satellite_position` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `number` varchar(10) DEFAULT '0',
  `packet_date` datetime NOT NULL DEFAULT '0000-00-00 00:00:00',
  `name` varchar(64) NOT NULL DEFAULT '',
  `elevation` int(11) NOT NULL DEFAULT '0',
  `azimuth` int(11) NOT NULL DEFAULT '0',
  `ra` double(11,6) NOT NULL DEFAULT '0.000000',
  `orbit_phase` int(11) NOT NULL DEFAULT '0',
  `latitude` decimal(11,6) NOT NULL DEFAULT '0.000000',
  `longitude` decimal(11,6) NOT NULL DEFAULT '0.000000',
  `slant_range` int(11) NOT NULL DEFAULT '0',
  `altitude` double(16,6) DEFAULT '0.000000',
  `orbit_num` int(11) NOT NULL DEFAULT '0',
  `sequence` int(11) NOT NULL,
  `create_ts` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`),
  KEY `packet_date` (`packet_date`),
  KEY `name` (`name`),
  KEY `create_ts` (`create_ts`),
  KEY `number` (`number`,`create_ts`)
) ENGINE=InnoDB AUTO_INCREMENT=336073938 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `save`
--

DROP TABLE IF EXISTS `save`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `save` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `user_id` int(11) unsigned NOT NULL DEFAULT '0',
  `hash` char(32) DEFAULT NULL,
  `name` varchar(64) DEFAULT NULL,
  `password` varchar(64) NOT NULL,
  `ip` varchar(15) NOT NULL,
  `zoom` int(10) NOT NULL DEFAULT '0',
  `longitude` double(9,6) NOT NULL DEFAULT '0.000000',
  `latitude` double(8,6) NOT NULL DEFAULT '0.000000',
  `access_ts` int(11) NOT NULL DEFAULT '0',
  `create_ts` int(14) DEFAULT '0',
  UNIQUE KEY `packet_id` (`id`),
  UNIQUE KEY `user_id` (`user_id`,`name`),
  KEY `packet_source` (`ip`),
  KEY `source` (`ip`),
  KEY `posted` (`create_ts`),
  KEY `create_ts` (`create_ts`),
  KEY `source_2` (`ip`,`create_ts`)
) ENGINE=InnoDB AUTO_INCREMENT=3323 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `saved_lastposition`
--

DROP TABLE IF EXISTS `saved_lastposition`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `saved_lastposition` (
  `packet_id` char(32) NOT NULL,
  `save_id` int(11) NOT NULL DEFAULT '0',
  `name` varchar(9) DEFAULT NULL,
  `source` varchar(9) NOT NULL DEFAULT '',
  `destination` varchar(9) NOT NULL DEFAULT '',
  `path` varchar(128) NOT NULL,
  `latitude` double(8,6) NOT NULL DEFAULT '0.000000',
  `longitude` double(9,6) NOT NULL DEFAULT '0.000000',
  `course` smallint(5) unsigned DEFAULT NULL,
  `speed` smallint(5) unsigned DEFAULT NULL,
  `altitude` mediumint(9) DEFAULT NULL,
  `status` tinytext,
  `symbol_table` char(1) DEFAULT NULL,
  `symbol_code` char(1) DEFAULT NULL,
  `type` enum('P','O','I') DEFAULT 'P',
  `create_ts` int(11) NOT NULL DEFAULT '0',
  KEY `latitude` (`latitude`),
  KEY `longitude` (`longitude`),
  KEY `packet_id` (`packet_id`),
  KEY `speed` (`speed`),
  KEY `save_id` (`save_id`),
  KEY `symbol_table` (`symbol_table`,`symbol_code`),
  KEY `destination` (`destination`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `saved_position`
--

DROP TABLE IF EXISTS `saved_position`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `saved_position` (
  `packet_id` char(32) NOT NULL,
  `save_id` int(11) NOT NULL DEFAULT '0',
  `source` char(9) NOT NULL DEFAULT '',
  `path` varchar(128) NOT NULL,
  `latitude` double(8,6) DEFAULT NULL,
  `longitude` double(9,6) DEFAULT NULL,
  `course` smallint(5) unsigned DEFAULT NULL,
  `speed` smallint(3) unsigned DEFAULT NULL,
  `altitude` mediumint(5) DEFAULT NULL,
  `symbol_table` char(1) NOT NULL,
  `symbol_code` char(1) NOT NULL,
  `create_ts` int(14) DEFAULT '0',
  UNIQUE KEY `packet_id` (`packet_id`),
  KEY `source` (`source`),
  KEY `posted` (`create_ts`),
  KEY `source_2` (`source`,`create_ts`),
  KEY `saved_id` (`save_id`,`source`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `session_message`
--

DROP TABLE IF EXISTS `session_message`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `session_message` (
  `user_id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `callsign` varchar(9) DEFAULT NULL,
  `num_message` int(11) NOT NULL DEFAULT '0',
  `create_ts` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`user_id`),
  KEY `callsign` (`callsign`,`create_ts`),
  KEY `creation` (`create_ts`)
) ENGINE=InnoDB AUTO_INCREMENT=39002 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `status`
--

DROP TABLE IF EXISTS `status`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `status` (
  `packet_id` int(10) unsigned NOT NULL DEFAULT '0',
  `callsign_id` int(11) NOT NULL,
  `source` varchar(9) NOT NULL DEFAULT '',
  `comment` tinytext,
  `power` tinyint(4) DEFAULT NULL,
  `height` smallint(5) unsigned DEFAULT NULL,
  `gain` tinyint(4) DEFAULT NULL,
  `directivity` tinyint(4) DEFAULT NULL,
  `rate` tinyint(4) DEFAULT NULL,
  `symbol_table` char(1) DEFAULT NULL,
  `symbol_code` char(1) DEFAULT NULL,
  PRIMARY KEY (`packet_id`),
  KEY `callsign_id` (`callsign_id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `statuses`
--

DROP TABLE IF EXISTS `statuses`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `statuses` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `hash` char(32) NOT NULL,
  `body` tinytext NOT NULL,
  `created` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`id`),
  UNIQUE KEY `hash` (`hash`),
  KEY `created` (`created`)
) ENGINE=InnoDB AUTO_INCREMENT=3126458 DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `task_state`
--

DROP TABLE IF EXISTS `task_state`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `task_state` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `name` varchar(64) NOT NULL,
  `updated_at` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (`id`),
  UNIQUE KEY `name_uniq` (`name`)
) ENGINE=InnoDB AUTO_INCREMENT=6 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `telemetry`
--

DROP TABLE IF EXISTS `telemetry`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `telemetry` (
  `packet_id` bigint(20) unsigned DEFAULT NULL,
  `callsign_id` int(11) NOT NULL,
  `source` char(10) NOT NULL DEFAULT '',
  `packet_date` timestamp NOT NULL DEFAULT '0000-00-00 00:00:00',
  `sequence` int(3) NOT NULL DEFAULT '0',
  `analog_0` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `analog_1` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `analog_2` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `analog_3` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `analog_4` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `digital` char(8) NOT NULL DEFAULT '00000000',
  `create_ts` int(14) NOT NULL DEFAULT '0',
  KEY `source` (`source`),
  KEY `create_ts` (`create_ts`),
  KEY `callsign_id` (`callsign_id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1
/*!50100 PARTITION BY RANGE ( UNIX_TIMESTAMP(packet_date))
(PARTITION 2017_10_16 VALUES LESS THAN (1508133600) ENGINE = InnoDB,
 PARTITION 2017_10_17 VALUES LESS THAN (1508220000) ENGINE = InnoDB,
 PARTITION 2017_10_18 VALUES LESS THAN (1508306400) ENGINE = InnoDB,
 PARTITION 2017_10_19 VALUES LESS THAN (1508392800) ENGINE = InnoDB,
 PARTITION 2017_10_20 VALUES LESS THAN (1508479200) ENGINE = InnoDB,
 PARTITION 2017_10_21 VALUES LESS THAN (1508565600) ENGINE = InnoDB,
 PARTITION 2017_10_22 VALUES LESS THAN (1508652000) ENGINE = InnoDB,
 PARTITION 2017_10_23 VALUES LESS THAN (1508738400) ENGINE = InnoDB,
 PARTITION 2017_10_24 VALUES LESS THAN (1508824800) ENGINE = InnoDB,
 PARTITION 2017_10_25 VALUES LESS THAN (1508911200) ENGINE = InnoDB,
 PARTITION 2017_10_26 VALUES LESS THAN (1508997600) ENGINE = InnoDB,
 PARTITION 2017_10_27 VALUES LESS THAN (1509084000) ENGINE = InnoDB,
 PARTITION 2017_10_28 VALUES LESS THAN (1509170400) ENGINE = InnoDB,
 PARTITION 2017_10_29 VALUES LESS THAN (1509256800) ENGINE = InnoDB,
 PARTITION 2017_10_30 VALUES LESS THAN (1509343200) ENGINE = InnoDB,
 PARTITION 2017_10_31 VALUES LESS THAN (1509429600) ENGINE = InnoDB,
 PARTITION 2017_11_01 VALUES LESS THAN (1509516000) ENGINE = InnoDB,
 PARTITION 2017_11_02 VALUES LESS THAN (1509602400) ENGINE = InnoDB,
 PARTITION 2017_11_03 VALUES LESS THAN (1509688800) ENGINE = InnoDB,
 PARTITION 2017_11_04 VALUES LESS THAN (1509775200) ENGINE = InnoDB) */;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `telemetry_bits`
--

DROP TABLE IF EXISTS `telemetry_bits`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `telemetry_bits` (
  `packet_id` bigint(20) unsigned DEFAULT NULL,
  `callsign_id` int(11) NOT NULL,
  `source` char(10) NOT NULL DEFAULT '',
  `bitsense` char(8) DEFAULT '00000000',
  `project_title` varchar(23) NOT NULL,
  `create_ts` int(14) NOT NULL DEFAULT '0',
  PRIMARY KEY (`callsign_id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `telemetry_eqns`
--

DROP TABLE IF EXISTS `telemetry_eqns`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `telemetry_eqns` (
  `packet_id` bigint(20) unsigned DEFAULT NULL,
  `callsign_id` int(11) NOT NULL,
  `source` char(10) NOT NULL DEFAULT '',
  `a_0` double DEFAULT NULL,
  `b_0` double DEFAULT NULL,
  `c_0` double DEFAULT NULL,
  `a_1` double DEFAULT NULL,
  `b_1` double DEFAULT NULL,
  `c_1` double DEFAULT NULL,
  `a_2` double DEFAULT NULL,
  `b_2` double DEFAULT NULL,
  `c_2` double DEFAULT NULL,
  `a_3` double DEFAULT NULL,
  `b_3` double DEFAULT NULL,
  `c_3` double DEFAULT NULL,
  `a_4` double DEFAULT NULL,
  `b_4` double DEFAULT NULL,
  `c_4` double DEFAULT NULL,
  `create_ts` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`callsign_id`),
  KEY `create_ts` (`create_ts`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `telemetry_parm`
--

DROP TABLE IF EXISTS `telemetry_parm`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `telemetry_parm` (
  `packet_id` bigint(20) unsigned DEFAULT NULL,
  `callsign_id` int(11) NOT NULL,
  `source` varchar(10) NOT NULL DEFAULT '',
  `a_0` varchar(7) DEFAULT NULL,
  `a_1` varchar(6) DEFAULT NULL,
  `a_2` varchar(5) DEFAULT NULL,
  `a_3` varchar(5) DEFAULT NULL,
  `a_4` varchar(4) DEFAULT NULL,
  `d_0` varchar(5) DEFAULT NULL,
  `d_1` varchar(4) DEFAULT NULL,
  `d_2` varchar(3) DEFAULT NULL,
  `d_3` varchar(3) DEFAULT NULL,
  `d_4` varchar(3) DEFAULT NULL,
  `d_5` varchar(2) DEFAULT NULL,
  `d_6` varchar(2) DEFAULT NULL,
  `d_7` varchar(2) DEFAULT NULL,
  `create_ts` int(11) DEFAULT '0',
  PRIMARY KEY (`callsign_id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `telemetry_unit`
--

DROP TABLE IF EXISTS `telemetry_unit`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `telemetry_unit` (
  `packet_id` bigint(20) unsigned DEFAULT NULL,
  `callsign_id` int(11) NOT NULL,
  `source` varchar(10) NOT NULL DEFAULT '',
  `a_0` varchar(7) DEFAULT NULL,
  `a_1` varchar(6) DEFAULT NULL,
  `a_2` varchar(5) DEFAULT NULL,
  `a_3` varchar(6) DEFAULT NULL,
  `a_4` varchar(4) DEFAULT NULL,
  `d_0` varchar(5) DEFAULT NULL,
  `d_1` varchar(4) DEFAULT NULL,
  `d_2` varchar(3) DEFAULT NULL,
  `d_3` varchar(3) DEFAULT NULL,
  `d_4` varchar(3) DEFAULT NULL,
  `d_5` varchar(2) DEFAULT NULL,
  `d_6` varchar(2) DEFAULT NULL,
  `d_7` varchar(2) DEFAULT NULL,
  `create_ts` int(11) DEFAULT '0',
  PRIMARY KEY (`callsign_id`),
  KEY `create_ts` (`create_ts`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `uls_am`
--

DROP TABLE IF EXISTS `uls_am`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `uls_am` (
  `record_type` char(2) DEFAULT NULL,
  `usi` int(9) NOT NULL DEFAULT '0',
  `uls` char(14) DEFAULT NULL,
  `ebf` varchar(30) DEFAULT NULL,
  `callsign` char(10) DEFAULT NULL,
  `class` char(1) DEFAULT NULL,
  `group_code` char(1) DEFAULT NULL,
  `region_code` tinyint(4) DEFAULT NULL,
  `trustee_callsign` char(10) DEFAULT NULL,
  `trustee_indicator` char(1) DEFAULT NULL,
  `physician_cert` char(1) DEFAULT NULL,
  `ve_signature` char(1) DEFAULT NULL,
  `sys_call_change` char(1) DEFAULT NULL,
  `vanity_call_change` char(1) DEFAULT NULL,
  `vanity_relationship` char(12) DEFAULT NULL,
  `previous_callsign` char(10) DEFAULT NULL,
  `previous_class` char(1) DEFAULT NULL,
  `trustee_name` varchar(50) DEFAULT NULL,
  PRIMARY KEY (`usi`),
  KEY `callsign` (`callsign`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `uls_en`
--

DROP TABLE IF EXISTS `uls_en`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `uls_en` (
  `record_type` char(2) DEFAULT NULL,
  `usi` int(9) NOT NULL DEFAULT '0',
  `uls` char(14) DEFAULT NULL,
  `ebf` varchar(30) DEFAULT NULL,
  `callsign` char(10) DEFAULT NULL,
  `entity_type` char(2) DEFAULT NULL,
  `licensee_id` char(9) DEFAULT NULL,
  `entity_name` varchar(200) DEFAULT NULL,
  `first_name` varchar(20) DEFAULT NULL,
  `middle_initial` char(1) DEFAULT NULL,
  `last_name` varchar(20) DEFAULT NULL,
  `suffix` char(3) DEFAULT NULL,
  `phone` char(10) DEFAULT NULL,
  `fax` char(10) DEFAULT NULL,
  `email` varchar(50) DEFAULT NULL,
  `street_address` varchar(60) DEFAULT NULL,
  `city` varchar(20) DEFAULT NULL,
  `state` char(2) DEFAULT NULL,
  `zip_code` char(9) DEFAULT NULL,
  `po_box` varchar(20) DEFAULT NULL,
  `attention_line` varchar(35) DEFAULT NULL,
  `sgin` char(3) DEFAULT NULL,
  `frn` char(10) DEFAULT NULL,
  `app_type_code` char(1) DEFAULT NULL,
  `app_type_other` char(40) DEFAULT NULL,
  `status_code` char(1) DEFAULT NULL,
  `status_date` char(10) DEFAULT NULL,
  PRIMARY KEY (`usi`),
  KEY `zip_code` (`zip_code`),
  KEY `location` (`city`,`state`),
  KEY `callsign` (`callsign`),
  KEY `frn` (`frn`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `uls_hd`
--

DROP TABLE IF EXISTS `uls_hd`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `uls_hd` (
  `record_type` char(2) DEFAULT NULL,
  `usi` int(9) NOT NULL DEFAULT '0',
  `uls` char(14) DEFAULT NULL,
  `ebf` varchar(30) DEFAULT NULL,
  `callsign` char(10) DEFAULT NULL,
  `license_status` char(1) DEFAULT NULL,
  `radio_service_code` char(2) DEFAULT NULL,
  `grant_date` char(10) DEFAULT NULL,
  `expired_date` char(10) DEFAULT NULL,
  `cancel_date` char(10) DEFAULT NULL,
  `eligibility_rule_num` char(10) DEFAULT NULL,
  `reserved1` char(1) DEFAULT NULL,
  `alien` char(1) DEFAULT NULL,
  `alien_government` char(1) DEFAULT NULL,
  `alien_corporation` char(1) DEFAULT NULL,
  `alien_officer` char(1) DEFAULT NULL,
  `alien_control` char(1) DEFAULT NULL,
  `revoked` char(1) DEFAULT NULL,
  `convicted` char(1) DEFAULT NULL,
  `adjudged` char(1) DEFAULT NULL,
  `reserved2` char(1) DEFAULT NULL,
  `common_carrier` char(1) DEFAULT NULL,
  `non_common_carrier` char(1) DEFAULT NULL,
  `private_comm` char(1) DEFAULT NULL,
  `fixed` char(1) DEFAULT NULL,
  `mobile` char(1) DEFAULT NULL,
  `radio_location` char(1) DEFAULT NULL,
  `satellite` char(1) DEFAULT NULL,
  `sta` char(1) DEFAULT NULL,
  `interconnected_service` char(1) DEFAULT NULL,
  `cert_first_name` varchar(20) DEFAULT NULL,
  `cert_middle_initial` char(1) DEFAULT NULL,
  `cert_last_name` varchar(20) DEFAULT NULL,
  `cert_suffix` char(3) DEFAULT NULL,
  `cert_title` char(40) DEFAULT NULL,
  `female` char(1) DEFAULT NULL,
  `black` char(1) DEFAULT NULL,
  `native_american` char(1) DEFAULT NULL,
  `hawaiian` char(1) DEFAULT NULL,
  `asian` char(1) DEFAULT NULL,
  `white` char(1) DEFAULT NULL,
  `hispanic` char(1) DEFAULT NULL,
  `effective_date` char(10) DEFAULT NULL,
  `last_action_date` char(10) DEFAULT NULL,
  `auction_id` int(11) DEFAULT NULL,
  `broadcast_services` char(1) DEFAULT NULL,
  `band_manager` char(1) DEFAULT NULL,
  `broadcast_services_type` char(1) DEFAULT NULL,
  `alien_ruling` char(1) DEFAULT NULL,
  `licensee_name_change` char(1) DEFAULT NULL,
  PRIMARY KEY (`usi`),
  KEY `callsign_2` (`callsign`,`license_status`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `usgs_broadcast`
--

DROP TABLE IF EXISTS `usgs_broadcast`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `usgs_broadcast` (
  `id` char(32) NOT NULL,
  `src` char(2) DEFAULT NULL,
  `eqid` varchar(64) DEFAULT '0',
  `version` char(1) NOT NULL,
  `create_ts` int(11) DEFAULT '0',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `usgs_quakes`
--

DROP TABLE IF EXISTS `usgs_quakes`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `usgs_quakes` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `src` char(2) NOT NULL,
  `eqid` varchar(64) DEFAULT '0',
  `version` char(1) NOT NULL,
  `latitude` double(8,6) NOT NULL,
  `longitude` double(9,6) DEFAULT NULL,
  `magnitude` double(3,2) DEFAULT NULL,
  `depth` double(4,2) DEFAULT NULL,
  `nst` int(11) NOT NULL,
  `name` varchar(64) NOT NULL,
  `poll_ts` int(11) DEFAULT '0',
  `create_ts` int(11) DEFAULT '0',
  PRIMARY KEY (`id`),
  KEY `magnitude` (`magnitude`),
  KEY `latitude` (`latitude`,`longitude`)
) ENGINE=InnoDB AUTO_INCREMENT=227370519 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `watch_programs`
--

DROP TABLE IF EXISTS `watch_programs`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `watch_programs` (
  `program` tinyint(4) unsigned NOT NULL DEFAULT '0',
  `user` varchar(20) DEFAULT NULL,
  `directory` text,
  `command` text NOT NULL,
  PRIMARY KEY (`program`),
  UNIQUE KEY `program` (`program`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='Listing of programs that can be triggered ';
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `watch_users`
--

DROP TABLE IF EXISTS `watch_users`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `watch_users` (
  `source` char(9) NOT NULL DEFAULT '',
  `program` tinyint(4) unsigned NOT NULL DEFAULT '0',
  KEY `source` (`source`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='Callsigns that trigger external programs';
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `weather`
--

DROP TABLE IF EXISTS `weather`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `weather` (
  `packet_id` bigint(20) unsigned DEFAULT NULL,
  `callsign_id` int(11) NOT NULL,
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
  KEY `create_ts` (`create_ts`),
  KEY `callsign_id` (`callsign_id`),
  KEY `packet_date` (`packet_date`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `weather_meta`
--

DROP TABLE IF EXISTS `weather_meta`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `weather_meta` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `callsign_id` int(11) NOT NULL,
  `last_rollup` timestamp NOT NULL DEFAULT '0000-00-00 00:00:00',
  `created` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `create_ts` int(11) NOT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `callsign_id` (`callsign_id`),
  KEY `last_rollup` (`last_rollup`),
  KEY `callsign_rollup_idx` (`callsign_id`,`last_rollup`)
) ENGINE=InnoDB AUTO_INCREMENT=25255176 DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `weather_rollup`
--

DROP TABLE IF EXISTS `weather_rollup`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `weather_rollup` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `callsign_id` int(11) NOT NULL,
  `num_rolled` int(11) NOT NULL,
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
  KEY `create_ts` (`create_ts`),
  KEY `callsign_id` (`callsign_id`),
  KEY `packet_date` (`packet_date`)
) ENGINE=InnoDB AUTO_INCREMENT=173660485 DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `web_ads`
--

DROP TABLE IF EXISTS `web_ads`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `web_ads` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `name` varchar(64) NOT NULL,
  `image` varchar(64) DEFAULT NULL,
  `title` varchar(128) DEFAULT NULL,
  `url` varchar(128) DEFAULT NULL,
  `target` varchar(64) NOT NULL DEFAULT '_new',
  `hits` int(11) NOT NULL DEFAULT '0',
  `clicks` int(11) NOT NULL DEFAULT '0',
  `hit_ts` int(11) NOT NULL DEFAULT '0',
  `click_ts` int(11) NOT NULL DEFAULT '0',
  `expire_ts` int(11) NOT NULL DEFAULT '0',
  `enable_banner` enum('Y','N') NOT NULL DEFAULT 'N',
  `active` enum('Y','N') NOT NULL DEFAULT 'Y',
  `create_ts` int(10) unsigned DEFAULT '0',
  PRIMARY KEY (`id`),
  UNIQUE KEY `name` (`name`),
  KEY `active` (`active`)
) ENGINE=InnoDB AUTO_INCREMENT=9 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `web_ajax`
--

DROP TABLE IF EXISTS `web_ajax`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `web_ajax` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `name` varchar(64) DEFAULT NULL,
  `title` varchar(64) NOT NULL,
  `variables` longtext NOT NULL,
  `description` longtext NOT NULL,
  `url` varchar(64) NOT NULL,
  `visible` enum('Y','N') NOT NULL DEFAULT 'Y',
  `create_ts` int(10) unsigned DEFAULT '0',
  PRIMARY KEY (`id`),
  UNIQUE KEY `name` (`name`),
  KEY `variable` (`name`)
) ENGINE=InnoDB AUTO_INCREMENT=28 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `web_ajax_examples`
--

DROP TABLE IF EXISTS `web_ajax_examples`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `web_ajax_examples` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `ajax_id` int(10) DEFAULT '0',
  `url` longtext,
  `create_ts` int(10) unsigned DEFAULT '0',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=38 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `web_ajax_restrictions`
--

DROP TABLE IF EXISTS `web_ajax_restrictions`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `web_ajax_restrictions` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `ajax_id` int(10) DEFAULT '0',
  `name` varchar(64) DEFAULT NULL,
  `value` longtext,
  `create_ts` int(10) unsigned DEFAULT '0',
  PRIMARY KEY (`id`),
  KEY `variable` (`name`)
) ENGINE=InnoDB AUTO_INCREMENT=205 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `web_ajax_variables`
--

DROP TABLE IF EXISTS `web_ajax_variables`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `web_ajax_variables` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `title` varchar(64) DEFAULT NULL,
  `name` varchar(64) NOT NULL,
  `field_type` enum('TEXT','INTEGER','ENUM') NOT NULL,
  `default_value` varchar(63) NOT NULL,
  `url` varchar(64) DEFAULT NULL,
  `description` longtext NOT NULL,
  `create_ts` int(10) unsigned DEFAULT '0',
  PRIMARY KEY (`id`),
  UNIQUE KEY `name` (`name`)
) ENGINE=InnoDB AUTO_INCREMENT=64 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `web_callsign`
--

DROP TABLE IF EXISTS `web_callsign`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `web_callsign` (
  `callsign` varchar(10) NOT NULL DEFAULT '',
  `create_ts` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`callsign`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `web_chat`
--

DROP TABLE IF EXISTS `web_chat`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `web_chat` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `ip` varchar(64) NOT NULL,
  `nick` varchar(64) DEFAULT NULL,
  `message` longtext NOT NULL,
  `sequence` int(11) NOT NULL DEFAULT '0',
  `create_ts` int(10) unsigned DEFAULT '0',
  PRIMARY KEY (`id`),
  KEY `sequence` (`sequence`),
  KEY `create_ts` (`create_ts`),
  KEY `nick` (`nick`)
) ENGINE=InnoDB AUTO_INCREMENT=1225 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `web_contact`
--

DROP TABLE IF EXISTS `web_contact`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `web_contact` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `name` varchar(128) NOT NULL DEFAULT '',
  `email` varchar(64) NOT NULL,
  `subject` varchar(128) DEFAULT NULL,
  `msg` longtext,
  `send_email` enum('Y','N') NOT NULL DEFAULT 'N',
  `create_ts` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=2277 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `web_dcc`
--

DROP TABLE IF EXISTS `web_dcc`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `web_dcc` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `short` varchar(64) DEFAULT NULL,
  `name` varchar(64) DEFAULT NULL,
  `title` varchar(128) DEFAULT NULL,
  `syntax` longtext,
  `type` enum('field','reply','cmd') DEFAULT 'reply',
  `create_ts` int(10) unsigned DEFAULT '0',
  PRIMARY KEY (`id`),
  UNIQUE KEY `short` (`short`,`type`),
  UNIQUE KEY `name` (`name`,`type`),
  KEY `type` (`type`)
) ENGINE=InnoDB AUTO_INCREMENT=183 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `web_dcc_access`
--

DROP TABLE IF EXISTS `web_dcc_access`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `web_dcc_access` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `name` varchar(64) NOT NULL DEFAULT '',
  `client` varchar(64) NOT NULL DEFAULT '',
  `hash` varchar(64) NOT NULL DEFAULT '',
  `autoverify` enum('Y','N') NOT NULL DEFAULT 'N',
  `active` enum('Y','N') NOT NULL DEFAULT 'N',
  PRIMARY KEY (`id`),
  UNIQUE KEY `client` (`client`),
  KEY `Client_Hash` (`client`,`hash`)
) ENGINE=InnoDB AUTO_INCREMENT=3 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `web_dcc_bans`
--

DROP TABLE IF EXISTS `web_dcc_bans`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `web_dcc_bans` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `uid` varchar(64) NOT NULL DEFAULT '',
  `reason` varchar(255) NOT NULL DEFAULT '',
  `active` enum('Y','N') NOT NULL DEFAULT 'Y',
  PRIMARY KEY (`id`),
  UNIQUE KEY `uid` (`uid`)
) ENGINE=InnoDB AUTO_INCREMENT=50 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `web_email_cache`
--

DROP TABLE IF EXISTS `web_email_cache`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `web_email_cache` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `callsign` varchar(10) NOT NULL DEFAULT '',
  `hash` char(32) NOT NULL,
  `create_ts` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`),
  UNIQUE KEY `callsign` (`callsign`,`hash`)
) ENGINE=InnoDB AUTO_INCREMENT=95938 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `web_email_ignore`
--

DROP TABLE IF EXISTS `web_email_ignore`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `web_email_ignore` (
  `callsign` varchar(10) NOT NULL DEFAULT '',
  `reason` varchar(128) NOT NULL,
  `create_ts` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`callsign`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `web_embed`
--

DROP TABLE IF EXISTS `web_embed`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `web_embed` (
  `ip` varchar(15) NOT NULL DEFAULT '',
  `site` varchar(64) NOT NULL,
  `timezone` varchar(64) NOT NULL,
  `count` int(11) DEFAULT '0',
  `visible` enum('Y','N') DEFAULT 'N',
  `update_ts` int(11) NOT NULL DEFAULT '0',
  `create_ts` int(10) unsigned DEFAULT '0',
  PRIMARY KEY (`site`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `web_friends`
--

DROP TABLE IF EXISTS `web_friends`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `web_friends` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `user_id` int(11) NOT NULL DEFAULT '0',
  `callsign` varchar(10) DEFAULT NULL,
  `aprs_call` varchar(9) NOT NULL,
  `seen_ts` int(11) DEFAULT '0',
  `create_ts` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`),
  UNIQUE KEY `user_id` (`user_id`,`callsign`),
  KEY `callsign` (`callsign`)
) ENGINE=InnoDB AUTO_INCREMENT=7609 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `web_graphs`
--

DROP TABLE IF EXISTS `web_graphs`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `web_graphs` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `type` enum('weather','telemetry') NOT NULL DEFAULT 'weather',
  `units` enum('imperial','metric','nautical') NOT NULL DEFAULT 'metric',
  `title` varchar(64) NOT NULL,
  `name` varchar(64) NOT NULL,
  `format` varchar(64) NOT NULL,
  `measure` varchar(64) NOT NULL,
  `priority` int(10) unsigned NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=19 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `web_help`
--

DROP TABLE IF EXISTS `web_help`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `web_help` (
  `id` char(32) NOT NULL,
  `name` varchar(128) NOT NULL DEFAULT '',
  `title` varchar(128) NOT NULL,
  `html` longtext,
  `site` enum('openaprs','oaprs','tools','tips','aprsmail','sarsec') NOT NULL DEFAULT 'openaprs',
  `by` varchar(64) NOT NULL,
  `tags` longtext NOT NULL,
  `filename` varchar(64) NOT NULL,
  `surveyq` longtext,
  `hits` int(11) NOT NULL DEFAULT '0',
  `help_yes` int(11) NOT NULL DEFAULT '0',
  `help_no` int(11) NOT NULL DEFAULT '0',
  `stars` int(11) NOT NULL DEFAULT '0',
  `hits_stars` int(11) NOT NULL DEFAULT '0',
  `ishelpful` enum('Y','N') DEFAULT 'Y',
  `create_ts` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`name`,`site`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `web_help_survey`
--

DROP TABLE IF EXISTS `web_help_survey`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `web_help_survey` (
  `id` varchar(32) NOT NULL,
  `help_id` char(32) NOT NULL DEFAULT '',
  `priority` int(11) NOT NULL DEFAULT '0',
  `name` varchar(64) NOT NULL,
  `title` varchar(128) NOT NULL,
  `hits` int(11) DEFAULT '0',
  `create_ts` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`help_id`,`title`),
  UNIQUE KEY `id` (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `web_help_why`
--

DROP TABLE IF EXISTS `web_help_why`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `web_help_why` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `help_id` char(32) NOT NULL,
  `why` longtext,
  `email_ts` int(11) NOT NULL DEFAULT '0',
  `create_ts` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=68 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `web_ie_whatsnew`
--

DROP TABLE IF EXISTS `web_ie_whatsnew`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `web_ie_whatsnew` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `name` varchar(64) NOT NULL DEFAULT '',
  `value` longtext,
  `create_ts` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=4 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `web_locations`
--

DROP TABLE IF EXISTS `web_locations`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `web_locations` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `user_id` int(11) NOT NULL DEFAULT '0',
  `name` varchar(32) DEFAULT NULL,
  `latitude` decimal(11,6) NOT NULL DEFAULT '0.000000',
  `longitude` decimal(11,6) NOT NULL DEFAULT '0.000000',
  `def` enum('Y','N') DEFAULT 'N',
  `create_ts` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`),
  UNIQUE KEY `user_id` (`user_id`,`name`),
  KEY `user_id_2` (`user_id`,`def`)
) ENGINE=InnoDB AUTO_INCREMENT=824 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `web_paypal_log`
--

DROP TABLE IF EXISTS `web_paypal_log`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `web_paypal_log` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `log_date` varchar(64) DEFAULT NULL,
  `message` longtext,
  `vars` longtext,
  `create_ts` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=2155 DEFAULT CHARSET=latin1 MAX_ROWS=1000000000;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `web_paypal_txn`
--

DROP TABLE IF EXISTS `web_paypal_txn`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `web_paypal_txn` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `user_id` int(11) NOT NULL DEFAULT '0',
  `txn_id` varchar(64) NOT NULL,
  `payment_status` varchar(64) NOT NULL,
  `payment_date` varchar(64) NOT NULL,
  `firstname` varchar(64) NOT NULL,
  `lastname` varchar(64) NOT NULL,
  `callsign` varchar(64) NOT NULL,
  `email` varchar(64) NOT NULL,
  `msg` varchar(64) NOT NULL,
  `create_ts` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`),
  UNIQUE KEY `txn_id` (`txn_id`),
  KEY `icon` (`callsign`),
  KEY `symbol_table` (`txn_id`,`callsign`)
) ENGINE=InnoDB AUTO_INCREMENT=1973 DEFAULT CHARSET=latin1 MAX_ROWS=1000000000;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `web_seeds`
--

DROP TABLE IF EXISTS `web_seeds`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `web_seeds` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `user_id` int(11) NOT NULL DEFAULT '0',
  `seed` char(64) DEFAULT '0',
  `ip` varchar(15) NOT NULL,
  `logged_in` enum('Y','N') NOT NULL DEFAULT 'N',
  `create_ts` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`),
  UNIQUE KEY `seed_2` (`seed`),
  KEY `seed` (`seed`)
) ENGINE=InnoDB AUTO_INCREMENT=1394631 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `web_send_email`
--

DROP TABLE IF EXISTS `web_send_email`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `web_send_email` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `mailfrom` varchar(128) NOT NULL DEFAULT '',
  `mailto` varchar(128) NOT NULL,
  `subject` varchar(128) DEFAULT NULL,
  `msg` longtext,
  `send_email` enum('Y','N') NOT NULL DEFAULT 'N',
  `create_ts` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`),
  KEY `send_email` (`send_email`)
) ENGINE=InnoDB AUTO_INCREMENT=99 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `web_stats`
--

DROP TABLE IF EXISTS `web_stats`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `web_stats` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `name` varchar(64) DEFAULT NULL,
  `count` double(10,2) unsigned DEFAULT '0.00',
  `create_ts` int(10) unsigned DEFAULT '0',
  PRIMARY KEY (`id`),
  KEY `name` (`name`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `web_tactical`
--

DROP TABLE IF EXISTS `web_tactical`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `web_tactical` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `user_id` int(11) NOT NULL DEFAULT '0',
  `callsign` varchar(10) DEFAULT NULL,
  `tactical` varchar(64) NOT NULL,
  `seen_ts` int(11) DEFAULT '0',
  `create_ts` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`),
  UNIQUE KEY `user_id` (`user_id`,`callsign`),
  KEY `callsign` (`callsign`)
) ENGINE=InnoDB AUTO_INCREMENT=1162 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `web_timezone`
--

DROP TABLE IF EXISTS `web_timezone`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `web_timezone` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `code` varchar(64) NOT NULL,
  `coordinates` varchar(64) DEFAULT NULL,
  `zone` longtext,
  `comments` longtext NOT NULL,
  `create_ts` int(10) unsigned DEFAULT '0',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=400 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `web_users`
--

DROP TABLE IF EXISTS `web_users`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `web_users` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `callsign` varchar(10) DEFAULT NULL,
  `first` varchar(64) DEFAULT NULL,
  `last` varchar(64) NOT NULL DEFAULT '',
  `email` varchar(64) NOT NULL DEFAULT '',
  `pass` varchar(64) NOT NULL DEFAULT '',
  `md5` varchar(64) NOT NULL DEFAULT '',
  `activate` varchar(64) NOT NULL DEFAULT '',
  `email_activate` enum('Y','N') NOT NULL DEFAULT 'N',
  `email_password` enum('Y','N') NOT NULL DEFAULT 'N',
  `verify` char(8) DEFAULT NULL,
  `verify_ts` int(11) DEFAULT '0',
  `msgchecksum` char(32) NOT NULL,
  `login_ts` int(11) NOT NULL DEFAULT '0',
  `create_ts` int(11) NOT NULL DEFAULT '0',
  `verified_ts` int(11) NOT NULL DEFAULT '0',
  `email_verified` enum('Y','N') NOT NULL DEFAULT 'N',
  `active` enum('Y','N') NOT NULL DEFAULT 'N',
  `active_until` int(11) NOT NULL DEFAULT '0',
  `verified` enum('Y','N') NOT NULL DEFAULT 'N',
  `paid` enum('Y','N') NOT NULL DEFAULT 'N',
  `paid_ts` int(11) NOT NULL DEFAULT '0',
  `ie` enum('Y','N') NOT NULL DEFAULT 'N',
  `guest` enum('Y','N') NOT NULL DEFAULT 'N',
  `local` enum('Y','N') NOT NULL DEFAULT 'N',
  `admin` enum('Y','N') NOT NULL DEFAULT 'N',
  PRIMARY KEY (`id`),
  UNIQUE KEY `email` (`email`),
  KEY `activate_active_idx` (`activate`,`active`)
) ENGINE=InnoDB AUTO_INCREMENT=39003 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `web_users_defaults`
--

DROP TABLE IF EXISTS `web_users_defaults`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `web_users_defaults` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `name` varchar(64) NOT NULL DEFAULT '',
  `value` longtext NOT NULL,
  `create_ts` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=6 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `web_users_options`
--

DROP TABLE IF EXISTS `web_users_options`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `web_users_options` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `user_id` int(11) NOT NULL DEFAULT '0',
  `name` varchar(64) NOT NULL DEFAULT '',
  `value` longtext,
  `create_ts` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=34873 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `web_users_watch_signups`
--

DROP TABLE IF EXISTS `web_users_watch_signups`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `web_users_watch_signups` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `name` varchar(64) NOT NULL DEFAULT '',
  `value` longtext,
  `check_ts` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=2 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `web_who`
--

DROP TABLE IF EXISTS `web_who`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `web_who` (
  `session` char(32) NOT NULL DEFAULT '',
  `server` varchar(64) NOT NULL,
  `ip` varchar(15) NOT NULL DEFAULT '',
  `nick` varchar(15) DEFAULT NULL,
  `timezone` varchar(64) NOT NULL,
  `offset_ts` int(11) NOT NULL DEFAULT '0',
  `idle` int(11) NOT NULL DEFAULT '0',
  `create_ts` int(11) unsigned DEFAULT '0',
  PRIMARY KEY (`session`),
  KEY `timezone` (`timezone`),
  KEY `create_ts` (`create_ts`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `web_xml`
--

DROP TABLE IF EXISTS `web_xml`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `web_xml` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `title` varchar(64) DEFAULT NULL,
  `name` varchar(64) NOT NULL,
  `url` varchar(64) DEFAULT NULL,
  `description` longtext NOT NULL,
  `create_ts` int(10) unsigned DEFAULT '0',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=8 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `web_xml_examples`
--

DROP TABLE IF EXISTS `web_xml_examples`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `web_xml_examples` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `xml_id` int(10) DEFAULT '0',
  `url` longtext,
  `create_ts` int(10) unsigned DEFAULT '0',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=27 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `web_xml_variables`
--

DROP TABLE IF EXISTS `web_xml_variables`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `web_xml_variables` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `xml_id` int(10) DEFAULT '0',
  `variable` varchar(64) NOT NULL,
  `field_type` enum('TEXT','INTEGER','ENUM') DEFAULT 'INTEGER',
  `default_value` varchar(64) NOT NULL,
  `description` longtext,
  `url` varchar(64) NOT NULL,
  `create_ts` int(10) unsigned DEFAULT '0',
  PRIMARY KEY (`id`),
  KEY `variable` (`variable`)
) ENGINE=InnoDB AUTO_INCREMENT=18 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `zipcodes`
--

DROP TABLE IF EXISTS `zipcodes`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `zipcodes` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `zip` int(11) NOT NULL DEFAULT '0',
  `latitude` double(8,6) NOT NULL DEFAULT '0.000000',
  `longitude` double(9,6) NOT NULL DEFAULT '0.000000',
  `city` varchar(64) NOT NULL DEFAULT '',
  `state` varchar(64) NOT NULL DEFAULT '',
  `county` varchar(64) NOT NULL DEFAULT '',
  `zip_class` varchar(64) NOT NULL DEFAULT '',
  PRIMARY KEY (`id`),
  KEY `zip` (`zip`),
  KEY `latitude` (`latitude`),
  KEY `longitude` (`longitude`)
) ENGINE=InnoDB AUTO_INCREMENT=42742 DEFAULT CHARSET=latin1 MAX_ROWS=1000000000;
/*!40101 SET character_set_client = @saved_cs_client */;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- Dump completed on 2017-10-25  9:25:01
