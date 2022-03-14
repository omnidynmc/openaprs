DROP PROCEDURE IF EXISTS getChatBySequence;
DELIMITER //
CREATE PROCEDURE getChatBySequence (seq INTEGER)
BEGIN
  SELECT t.nick,
         t.message,
         t.sequence,
         t.create_ts,
         FROM_UNIXTIME(t.create_ts, '%c/%e %l:%i %p') AS post_date
    FROM web_chat t
   WHERE t.sequence >= seq
   ORDER BY t.sequence DESC,
            t.create_ts DESC
   LIMIT 20;
END //
DELIMITER ;
