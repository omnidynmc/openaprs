DELIMITER ;;
CREATE PROCEDURE `partition_drop`(IN through_date date, IN tbl varchar(64), IN db varchar(64))
BEGIN
DECLARE delete_me varchar(64);
DECLARE notfound BOOL DEFAULT FALSE;
DECLARE pname CURSOR FOR SELECT PARTITION_NAME
FROM INFORMATION_SCHEMA.PARTITIONS WHERE TABLE_NAME=tbl AND TABLE_SCHEMA=db
AND DATE(PARTITION_NAME)!= 0
AND DATE(PARTITION_NAME) IS NOT NULL
AND DATE(PARTITION_NAME)<=through_date;

DECLARE CONTINUE HANDLER FOR NOT FOUND SET notfound:=TRUE;
OPEN pname;

cursor_loop: LOOP
FETCH pname INTO delete_me;
IF notfound THEN LEAVE cursor_loop; END IF;
SET @alter_stmt:=CONCAT("ALTER TABLE ",db,".",tbl," DROP PARTITION ",delete_me);

# sanity check commented out for production use
# SELECT @alter_stmt;
PREPARE stmt_alter FROM @alter_stmt; EXECUTE stmt_alter; DEALLOCATE PREPARE stmt_alter;

END LOOP;
CLOSE pname;
END ;;
DELIMITER ;
