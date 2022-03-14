DROP PROCEDURE IF EXISTS getSavedPositionsBySource;
DELIMITER //
CREATE PROCEDURE getSavedPositionsBySource (save_id BIGINT, source VARCHAR(9), age DATETIME)
BEGIN
  SELECT
    FROM_UNIXTIME(t.create_ts) AS packet_date,
    t.path,
    t.latitude,
    t.longitude,
    t.create_ts
  FROM
    saved_position t
  WHERE
    (t.save_id = save_id)
    AND (t.source = source)
  LIMIT 1;
END //
DELIMITER ;
