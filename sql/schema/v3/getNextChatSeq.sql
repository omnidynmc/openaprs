DROP PROCEDURE IF EXISTS getNextChatSeq;
DELIMITER //
CREATE PROCEDURE getNextChatSeq ()
BEGIN
  SELECT MAX(sequence)+1 AS sequence
    FROM web_chat;
END //
DELIMITER ;
