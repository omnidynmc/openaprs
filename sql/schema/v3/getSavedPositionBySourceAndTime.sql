DROP PROCEDURE IF EXISTS getSavedPositionBySourceAndTime;
DELIMITER //
CREATE PROCEDURE getSavedPositionBySourceAndTime (save_id BIGINT, source VARCHAR(9), seconds INT)
BEGIN
  SELECT
    t.packet_id,
    FROM_UNIXTIME(t.create_ts) AS packet_date,
    t.source,
    t.path,
    t.latitude,
    t.longitude,
    t.course,
    t.speed,
    t.altitude,
    t.symbol_table,
    t.symbol_code,
    IF(t.course > 0 AND ai.direction = 'Y',
      CONCAT(ai.icon_path, '/compass/', REPLACE(ai.image, '.png', ''), '-', getDirectionFromCourse(t.course), '.png'),
      CONCAT(ai.icon_path, '/', ai.image)
    ) AS icon,
    t.create_ts
  FROM
    saved_position t
    LEFT JOIN aprs_icons ai ON (BINARY ai.symbol_table=IF(t.symbol_table != '\\' && t.symbol_table != '/', '\\', t.symbol_table) AND BINARY ai.icon=t.symbol_code)
  WHERE
    t.source=source
    AND t.create_ts=seconds
  LIMIT 1;
END //
DELIMITER ;
