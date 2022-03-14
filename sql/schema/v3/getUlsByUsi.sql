DROP PROCEDURE IF EXISTS getUlsByUsi;
DELIMITER //
CREATE PROCEDURE getUlsByUsi (usi INTEGER)
BEGIN
  SELECT 
    hd.grant_date,
    hd.expired_date,
    hd.cancel_date,
    hd.callsign,
    getLicenseClassByToken(am.class) AS class,
    am.previous_callsign,
    am.previous_class,
    en.entity_name,
    en.first_name,
    en.middle_initial,
    en.last_name,
    en.street_address,
    en.po_box,
    en.city,
    en.state,
    en.zip_code,
    en.frn,
    hd.usi
  FROM 
    uls_hd hd
    LEFT JOIN uls_en en ON en.usi=hd.usi
    LEFT JOIN uls_am am ON am.usi=hd.usi 
  WHERE 
    hd.usi=usi
    AND hd.license_status='A' 
  LIMIT 1;
END //
DELIMITER ;
