DROP PROCEDURE IF EXISTS getMemPositionsBySource;
DELIMITER //
CREATE PROCEDURE getMemPositionsBySource (source VARCHAR(9), intv TIMESTAMP, lim INTEGER)
BEGIN
  SET @source = source;
  SET @intv = intv;
  SET @q = "
    SELECT
      t.latitude,
      t.longitude,
      UNIX_TIMESTAMP(t.packet_date) AS create_ts
    FROM
      callsign c
      LEFT JOIN mem_position t ON c.id=t.callsign_id
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
