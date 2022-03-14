DROP FUNCTION IF EXISTS convertUnitTo;
DELIMITER //
CREATE FUNCTION convertUnitTo (num FLOAT, unit VARCHAR(255))
RETURNS FLOAT
BEGIN
  DECLARE ret FLOAT DEFAULT 0.0;

  SET ret = CASE
    WHEN unit = 'KnotsToKph' THEN (num * 1.852)
    WHEN unit = 'KphToKnot' THEN (num * 0.539956803)
    WHEN unit = 'KphToMph' THEN (num * 0.621371192)
    WHEN unit = 'MphToKph' THEN (num * 1.609344)
    WHEN unit = 'KphToMs' THEN (num * (10 / 36))
    WHEN unit = 'MphToMs' THEN (num * (1.609344 * 10 / 36))
    WHEN unit = 'InchToMm' THEN (num * 0.254)
    WHEN unit = 'MmToInch' THEN (num * 0.0393700787)
    WHEN unit = 'MeterToFt' THEN (num * 3.2808399)
    WHEN unit = 'FtToMeter' THEN (num * 0.3048)
    WHEN unit = 'none' THEN (num)
    ELSE -1.0
  END;

  RETURN ret;
END //
DELIMITER ;
