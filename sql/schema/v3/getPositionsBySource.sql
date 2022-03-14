DROP PROCEDURE IF EXISTS getPositionsBySource;
DELIMITER //
CREATE PROCEDURE getPositionsBySource (source VARCHAR(9), intv TIMESTAMP, lim INTEGER)
BEGIN
  SET @source = source;
  SET @intv = intv;
  SET @q = "
    SELECT
      t.latitude,
      t.longitude,
      pa.body AS path,
      t.create_ts
    FROM
      callsign c
      LEFT JOIN position t ON c.id=t.callsign_id
      LEFT JOIN path pa ON pa.id=t.path_id
    WHERE
      c.source = ?
      AND t.packet_date >= ?
    ORDER BY
      t.packet_date DESC
    LIMIT ";

  SET @q = CONCAT(@q, lim);
  PREPARE query FROM @q;
  EXECUTE query USING @source, @intv;
  DEALLOCATE PREPARE query;
END //
DELIMITER ;
