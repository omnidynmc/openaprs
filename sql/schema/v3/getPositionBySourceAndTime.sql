DROP PROCEDURE IF EXISTS getPositionBySourceAndTime;
DELIMITER //
CREATE PROCEDURE getPositionBySourceAndTime (source VARCHAR(9), seconds INT)
BEGIN
  SELECT
    t.packet_id,
    t.packet_date,
    c.source,
    p.body AS path,
    t.latitude,
    t.longitude,
    t.course,
    t.speed,
    t.altitude,
    s.body AS status,
    t.symbol_table,
    t.symbol_code,
    IF(t.course > 0 AND ai.direction = 'Y',
      CONCAT(ai.icon_path, '/compass/', REPLACE(ai.image, '.png', ''), '-', getDirectionFromCourse(t.course), '.png'),
      CONCAT(ai.icon_path, '/', ai.image)
    ) AS icon,
    t.create_ts
  FROM
    callsign c
    LEFT JOIN position t ON c.id=t.callsign_id
    LEFT JOIN statuses s ON s.id=t.status_id
    LEFT JOIN path p ON p.id=t.path_id
    LEFT JOIN aprs_icons ai ON (BINARY ai.symbol_table=IF(t.symbol_table != '\\' && t.symbol_table != '/', '\\', t.symbol_table) AND BINARY ai.icon=t.symbol_code)
  WHERE
    c.source=source
    AND t.packet_date=FROM_UNIXTIME(seconds)
  LIMIT 1;
END //
DELIMITER ;
