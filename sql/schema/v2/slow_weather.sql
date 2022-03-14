UPDATE weather t SET t.callsign_id = (SELECT c.id FROM callsign c WHERE UPPER(c.source) = UPPER(t.source) LIMIT 1) WHERE t.callsign_id=0 LIMIT 100
