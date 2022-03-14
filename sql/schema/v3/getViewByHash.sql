DROP PROCEDURE IF EXISTS getViewByHash;
DELIMITER //
CREATE PROCEDURE getViewByHash (hash VARCHAR(32))
BEGIN
  SELECT
    t.id,
    t.name,
    t.zoom,
    t.latitude,
    t.longitude,
    t.create_ts
  FROM
    save t
  WHERE
    (t.hash = hash)
  LIMIT 1;
END //
DELIMITER ;
