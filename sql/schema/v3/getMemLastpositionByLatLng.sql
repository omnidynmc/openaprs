DROP PROCEDURE IF EXISTS getMemLastpositionByLatLng;
DELIMITER //
CREATE PROCEDURE getMemLastpositionByLatLng (lat1 FLOAT, lng1 FLOAT, lat2 FLOAT, lng2 FLOAT, intv TIMESTAMP, lim INTEGER)
BEGIN
  SET @lat1 = lat1;
  SET @lng1 = lng1;
  SET @lat2 = lat2;
  SET @lng2 = lng2;
  SET @intv = intv;
  SET @q = "

  SELECT lp.packet_id,
         c.source,
         lp.name,
         p.body AS path,
         lp.latitude,
         lp.longitude,
         lp.course,
         lp.symbol_table,
         lp.symbol_code,
         lp.phg_range,
         lp.phg_direction,
         lp.type,
         IF(lp.course > 0 AND ai.direction = 'Y',
           CONCAT(ai.icon_path, '/compass/', REPLACE(ai.image, '.png', ''), '-', getDirectionFromCourse(lp.course), '.png'),
           CONCAT(ai.icon_path, '/', ai.image)
         ) AS icon,
         UNIX_TIMESTAMP(lp.packet_date) AS create_ts
    FROM callsign c
         LEFT JOIN mem_lastposition lp ON c.id=lp.callsign_id
         LEFT JOIN path p ON p.id=lp.path_id
         LEFT JOIN aprs_icons ai ON (BINARY ai.symbol_table=IF(lp.symbol_table != '\\\\' && lp.symbol_table != '/', '\\\\', lp.symbol_table) AND BINARY ai.icon=lp.symbol_code)
   WHERE (lp.latitude <= ? AND lp.longitude >= ? AND lp.latitude >= ? AND lp.longitude <= ?)
     AND (lp.packet_date >= ?)
   LIMIT ";

  SET @q = CONCAT(@q, lim);
  PREPARE query FROM @q;
  EXECUTE query USING @lat1, @lng1, @lat2, @lng2, @intv;
  DEALLOCATE PREPARE query;
END //
DELIMITER ;
