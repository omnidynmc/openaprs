DROP PROCEDURE IF EXISTS getSavedLastpositionBySource;
DELIMITER //
CREATE PROCEDURE getSavedLastpositionBySource (save_id BIGINT, source VARCHAR(9))
BEGIN
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
    LEFT JOIN aprs_icons ai ON (BINARY ai.symbol_table=IF(lp.symbol_table != '\\' && lp.symbol_table != '/', '\\', lp.symbol_table) AND BINARY ai.icon=lp.symbol_code)
  WHERE
    (lp.source=source && lp.type = 'P')
    OR (lp.name=source && lp.type = 'O')
  LIMIT 1;
END //
DELIMITER ;
