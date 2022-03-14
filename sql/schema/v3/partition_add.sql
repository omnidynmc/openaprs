DELIMITER ;;
CREATE PROCEDURE `partition_add`(IN through_date date, IN tbl varchar(64), IN db varchar(64))
BEGIN
DECLARE add_me char(10);
DECLARE max_new_parts,add_cnt smallint unsigned default 0;
SELECT 1024-COUNT(*) AS max_new_parts,
SUM(CASE WHEN
 DATE(PARTITION_NAME)>=through_date then 1 else 0
 END)
INTO max_new_parts, add_cnt
FROM INFORMATION_SCHEMA.PARTITIONS
WHERE  TABLE_SCHEMA = db
 AND TABLE_NAME = tbl;

IF add_cnt=0 THEN
BEGIN
SELECT MAX(DATE(PARTITION_NAME)) INTO add_me
FROM INFORMATION_SCHEMA.PARTITIONS WHERE TABLE_NAME=tbl and TABLE_SCHEMA=db
AND DATE(PARTITION_NAME)<through_date;

# to do: declare handler for exceptions here
IF DATEDIFF(through_date,add_me)+1 < max_new_parts THEN
BEGIN
WHILE add_me<through_date do BEGIN
SET add_me:=DATE_FORMAT(add_me + INTERVAL 1 DAY,"%Y_%m_%d");
SET @alter_stmt:=CONCAT("ALTER TABLE ",db,".",tbl," ADD PARTITION (PARTITION ",add_me," VALUES LESS THAN (TO_DAYS('",add_me+INTERVAL 1 DAY, "')))" );

PREPARE stmt_alter FROM @alter_stmt; EXECUTE stmt_alter; DEALLOCATE PREPARE stmt_alter;

END;
END WHILE;
END;
END IF;
END;
END IF;
END ;;
DELIMITER ;
