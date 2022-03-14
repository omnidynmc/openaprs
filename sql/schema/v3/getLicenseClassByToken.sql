DROP FUNCTION IF EXISTS getLicenseClassBytoken;
DELIMITER //
CREATE FUNCTION getLicenseClassByToken (tok CHAR(1))
RETURNS VARCHAR(255)
BEGIN
  DECLARE ret VARCHAR(255) DEFAULT '';

  SET ret = CASE
    WHEN tok = 'A' THEN 'advanced'
    WHEN tok = 'T' THEN 'technician'
    WHEN tok = 'G' THEN 'general'
    WHEN tok = 'E' THEN 'extra'
    WHEN tok = 'N' THEN 'novice'
    WHEN tok = 'P' THEN 'technician plus'
    WHEN tok = 'C' THEN 'club'
    ELSE 'club'
  END;

  RETURN ret;
END //
DELIMITER ;
