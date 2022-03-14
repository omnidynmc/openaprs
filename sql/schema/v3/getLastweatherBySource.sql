DROP PROCEDURE IF EXISTS getLastweatherBySource;
DELIMITER //
CREATE PROCEDURE getLastweatherBySource (source VARCHAR(9))
BEGIN
  SELECT 
    t.packet_date,
    t.wind_speed,
    t.wind_gust,
    t.wind_sustained,
    t.temperature,
    t.rain_hour,
    t.rain_calendar_day,
    t.rain_24hour_day,
    t.humidity,
    t.barometer,
    t.luminosity,
    t.create_ts,
    ((1.8 * temperature) + 32) AS tempF, 
    c.source 
  FROM 
    lastweather t 
    LEFT JOIN callsign c ON c.id=t.callsign_id 
  WHERE 
    c.source = source 
  LIMIT 1;
END //
DELIMITER ;
