DROP PROCEDURE IF EXISTS setChat;
DELIMITER //
CREATE PROCEDURE setChat (i_ip VARCHAR(64), i_nick VARCHAR(64), i_message LONGTEXT, sequence INTEGER)
BEGIN
  INSERT
    INTO web_chat (ip, nick, message, sequence, create_ts)
  VALUES (i_ip, i_nick, i_message, i_sequence, UNIX_TIMESTAMP() );
END //
DELIMITER ;
