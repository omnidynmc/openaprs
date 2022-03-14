DROP PROCEDURE IF EXISTS getSavedLastpositionByLatLng;
DELIMITER //
CREATE PROCEDURE getSavedLastpositionByLatLng (save_id BIGINT, lat1 FLOAT, lng1 FLOAT, lat2 FLOAT, lng2 FLOAT, intv TIMESTAMP, lim INTEGER)
BEGIN
  SET @save_id = save_id;
  SET @lat1 = lat1;
  SET @lng1 = lng1;
  SET @lat2 = lat2;
  SET @lng2 = lng2;
  SET @intv = intv;
  SET @q = "
  SELECT
    lp.packet_id,
    FROM_UNIXTIME(lp.create_ts) AS packet_date,
    lp.source,
    lp.name,
    lp.destination,
    lp.path,
    lp.latitude,
    lp.longitude,
    lp.course,
    lp.speed,
    lp.altitude,
    lp.status,
    lp.symbol_table,
    lp.symbol_code,
    lp.type,
    IF(lp.course > 0 AND ai.direction = 'Y',
      CONCAT(ai.icon_path, '/compass/', REPLACE(ai.image, '.png', ''), '-', getDirectionFromCourse(lp.course), '.png'),
      CONCAT(ai.icon_path, '/', ai.image)
    ) AS icon,
    lp.create_ts
  FROM
    saved_lastposition lp
    LEFT JOIN aprs_icons ai ON (BINARY ai.symbol_table=IF(lp.symbol_table != '\\\\' && lp.symbol_table != '/', '\\\\', lp.symbol_table) AND BINARY ai.icon=lp.symbol_code)
  WHERE
    lp.save_id = ?
  LIMIT ";

--    AND (lp.latitude <= ? AND lp.longitude >= ? AND lp.latitude >= ? AND lp.longitude <= ?)
--    AND (lp.create_ts >= UNIX_TIMESTAMP(?))

  SET @q = CONCAT(@q, lim);
  PREPARE query FROM @q;
  EXECUTE query USING @save_id;
  DEALLOCATE PREPARE query;
END //
DELIMITER ;
