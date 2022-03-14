DROP PROCEDURE IF EXISTS convertForUnitAndSystem;
DELIMITER //
CREATE PROCEDURE convertForUnitAndSystem (IN num FLOAT, IN type VARCHAR(255), IN system VARCHAR(255), 
                                          OUT ret FLOAT, OUT suffix VARCHAR(255))
BEGIN
  DECLARE converter VARCHAR(255);

  CASE
    WHEN type = 'speed' AND system = 'metric' THEN
      SET converter = 'none';
      SET suffix = 'KPH';
    WHEN type = 'speed' AND system = 'imperial' THEN 
      SET converter = 'KphToMph';
      SET suffix = 'MPH';
    WHEN type = 'speed' AND system = 'nautical' THEN 
      SET converter = 'KphToKnot';
      SET suffix = 'Knots';

    WHEN type = 'vol' AND system = 'metric' THEN 
      SET converter = 'none';
      SET suffix = 'mm';
    WHEN type = 'vol' AND system = 'imperial' THEN 
      SET converter = 'MmToInch';
      SET suffix = 'in';
    WHEN type = 'vol' AND system = 'nautical' THEN 
      SET converter = 'MmToInch';
      SET suffix = 'in';

    WHEN type = 'dist' AND system = 'metric' THEN 
      SET converter = 'none';
      SET suffix = 'km';
    WHEN type = 'dist' AND system = 'imperial' THEN 
      SET converter = 'KphToMph';
      SET suffix = 'mi';
    WHEN type = 'dist' AND system = 'nautical' THEN 
      SET converter = 'KphToKnot';
      SET suffix = 'km';

    WHEN type = 'ruler' AND system = 'metric' THEN 
      SET converter = 'none';
      SET suffix = 'm';
    WHEN type = 'ruler' AND system = 'imperial' THEN 
      SET converter = 'MeterToFt';
      SET suffix = 'ft';
    WHEN type = 'ruler' AND system = 'nautical' THEN 
      SET converter = 'none';
      SET suffix = 'm';
  END CASE;

  SET ret = convertUnitTo(num, converter);
END //
DELIMITER ;
