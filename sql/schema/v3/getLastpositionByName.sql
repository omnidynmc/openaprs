DROP PROCEDURE IF EXISTS getLastpositionByName;
DELIMITER //
CREATE PROCEDURE getLastpositionByName (name VARCHAR(9))
BEGIN
  SELECT
    lp.packet_id,
    lp.packet_date,
    c.source,
    lp.name,
    lp.destination,
    p.body AS path,
    lp.latitude,
    lp.longitude,
    lp.course,
    lp.speed,
    lp.altitude,
    s.body AS status,
    lp.symbol_table,
    lp.symbol_code,
    lp.overlay,
    lp.phg_power,
    lp.phg_haat,
    lp.phg_gain,
    lp.phg_range,
    lp.phg_direction,
    lp.phg_beacon,
    lp.dfs_power,
    lp.dfs_gain,
    lp.dfs_direction,
    lp.range,
    lp.type,
    lp.weather,
    lp.telemetry,
    lp.igate,
    lp.postype,
    lp.mbits,
    IF(lp.course > 0 AND ai.direction = 'Y',
      CONCAT(ai.icon_path, '/compass/', REPLACE(ai.image, '.png', ''), '-', getDirectionFromCourse(lp.course), '.png'),
      CONCAT(ai.icon_path, '/', ai.image)
    ) AS icon,
    lp.create_ts
  FROM
    callsign c
    LEFT JOIN lastposition lp ON c.id=lp.callsign_id
    LEFT JOIN statuses s ON s.id=lp.status_id
    LEFT JOIN path p ON p.id=lp.path_id
    LEFT JOIN aprs_icons ai ON (BINARY ai.symbol_table=IF(lp.symbol_table != '\\' && lp.symbol_table != '/', '\\', lp.symbol_table) AND BINARY ai.icon=lp.symbol_code)
  WHERE
    (lp.name=name && lp.type = 'O')
  LIMIT 1;
END //
DELIMITER ;
