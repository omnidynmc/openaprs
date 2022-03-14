DROP PROCEDURE IF EXISTS getWhoByTime;
DELIMITER //
CREATE PROCEDURE getWhoByTime (from_time INTEGER)
BEGIN
  SELECT t.nick,
         t.ip,
         MIN(t.idle) AS idle,
         t.create_ts,
         COUNT(t.nick) AS count
    FROM web_who t
   WHERE t.create_ts >= from_time
   GROUP BY t.nick
   ORDER BY t.nick ASC;
END //
DELIMITER ;
