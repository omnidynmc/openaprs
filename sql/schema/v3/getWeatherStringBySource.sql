DROP PROCEDURE IF EXISTS getWeatherStringBySource;
DELIMITER //
CREATE PROCEDURE getWeatherStringBySource (source VARCHAR(9), units VARCHAR(255))
BEGIN
  SELECT 
    CONCAT(
      IF(t.temperature IS NULL,
        '',
        CONCAT(t.temperature,
               ' C / ',
               ((1.8 * temperature) + 32), 
               ' F '
        )
      ),
      IF(t.humidity IS NULL, '', CONCAT(t.humidity, '% ')),
      IF(t.barometer IS NULL, '', CONCAT(t.barometer, 'mbar ')),
      IF(t.wind_direction IS NULL, '-',
        CONCAT(t.wind_direction,
          ' ',
          getShortDirectionFromCourse(t.wind_direction))
      ),
      ' ',
      IF(t.wind_speed IS NULL, '-', convertUnitToString(t.wind_speed, 1, 'speed', units)),
      ' (gust: ',
      IF(t.wind_gust IS NULL, '-', convertUnitToString(t.wind_gust, 1, 'speed', units)),
      ')'
    ) AS weather
  FROM 
    lastweather t 
    LEFT JOIN callsign c ON c.id=t.callsign_id 
  WHERE 
    c.source = source 
  LIMIT 1;
END //
DELIMITER ;
