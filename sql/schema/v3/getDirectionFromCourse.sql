DROP FUNCTION IF EXISTS getDirectionFromCourse;
DELIMITER //
CREATE FUNCTION getDirectionFromCourse (course INTEGER)
RETURNS VARCHAR(255)
BEGIN
  DECLARE ret VARCHAR(255) DEFAULT '';
  DECLARE tmp VARCHAR(255) DEFAULT '';
  DECLARE dir INTEGER DEFAULT 0;

  DECLARE rounded INTEGER DEFAULT 0;
  SET rounded = round(course / 22.5) % 16;

  IF ((rounded mod 4) = 0) THEN
    SET ret = CASE
      WHEN (rounded / 4) = 0 THEN 'north'
      WHEN (rounded / 4) = 1 THEN 'east'
      WHEN (rounded / 4) = 2 THEN 'south'
      WHEN (rounded / 4) = 3 THEN 'west'
    END;
  ELSE
    SET dir = 2 * floor(((floor(rounded / 4) + 1) mod 4) / 2);
    SET ret = CASE
      WHEN dir = 0 THEN 'north'
      WHEN dir = 1 THEN 'east'
      WHEN dir = 2 THEN 'south'
      WHEN dir = 3 THEN 'west'
    END;
    SET dir = 1 + 2 * floor(rounded / 8);
    SET tmp = CASE
      WHEN dir = 0 THEN 'north'
      WHEN dir = 1 THEN 'east'
      WHEN dir = 2 THEN 'south'
      WHEN dir = 3 THEN 'west'
    END;

    SET ret = CONCAT(ret, '_', tmp);
  END IF;

  RETURN ret;
END //
DELIMITER ;
