DROP FUNCTION IF EXISTS convertUnitToString;
DELIMITER //
CREATE FUNCTION convertUnitToString (num FLOAT, prec INTEGER, type VARCHAR(255), system VARCHAR(255))
RETURNS VARCHAR(255)
BEGIN
  DECLARE ret VARCHAR(255);

  CALL convertForUnitAndSystem(num, type, system, @converted, @suffix);

  SET ret = CONCAT(ROUND(@converted, prec), ' ', @suffix);

  RETURN ret;
END //
DELIMITER ;
