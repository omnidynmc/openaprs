DROP PROCEDURE IF EXISTS setWebWho;
DELIMITER //
CREATE PROCEDURE setWebWho (hash VARCHAR(32), server VARCHAR(64), ip VARCHAR(15), nick VARCHAR(64),
                            timezone VARCHAR(64), offset_ts INT, idle INT)
BEGIN
  INSERT
    INTO web_who (session, server, ip, nick, timezone, offset_ts, idle, create_ts) 
  VALUES (hash, server, ip, UPPER(nick), timezone, offset_ts, idle, UNIX_TIMESTAMP())
  ON DUPLICATE KEY UPDATE 
         session=VALUES(session),
         server=VALUES(server),
         ip=VALUES(ip),
         timezone=VALUES(timezone),
         offset_ts=VALUES(offset_ts),
         idle=VALUES(idle),
         create_ts=VALUES(create_ts);
END //
DELIMITER ;
