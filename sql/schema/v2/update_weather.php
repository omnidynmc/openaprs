#!/usr/bin/php
<?php
  $cfg['mysql_host'] = "localhost";
  $cfg['mysql_user'] = "root";
  $cfg['mysql_pass'] = "";
  $cfg['mysql_db'] = "openaprs";

  $mysql = mysql_connect($cfg['mysql_host'], $cfg['mysql_user'], $cfg['mysql_pass']);

  if (!$mysql)
    die("ERROR: Unable to connect to mysql; ".mysql_error());

  if (!mysql_select_db($cfg['mysql_db']))
    die("ERROR: Unable to use db; ".mysql_error());

  $day = 30;

  while(1) {
    $day--;
    echo "DAY($day) PREV($prev)\n";
    $prev = $day + 1;
    $sqls = "SELECT DISTINCT source FROM weather WHERE callsign_id=0 AND packet_date <= DATE(NOW()) - INTERVAL $day DAY AND packet_date >= DATE(NOW()) - INTERVAL $prev DAY";
    $numRows = fetch($sqls, $ret);
    if ($numRows < 1 && $day == 1) {
      echo "No rows found.\n";
      break;
    } # if

    echo "Selected " . $numRows . " rows\n";

    foreach($ret AS $row) {
      $sqls ="SELECT id FROM callsign WHERE source='$row[source]'";
      $num = fetchRow($sqls, $call);
      if ($num < 1)
        continue;

      echo "Found id " . $call[id] . " for " . $row[source] . "\n";
      $sqls = "UPDATE weather SET callsign_id='$call[id]' WHERE source='$row[source]' AND packet_date <= DATE(NOW()) - INTERVAL $day DAY AND packet_date >= DATE(NOW()) - INTERVAL $prev DAY";
      $num = update($sqls);
      echo "Updated " . $num . " rows for " . $row[source] . "\n";
    } // foreach
  } // while

  mysql_close($mysql);

  function fetch($s, &$ret) {
    global $mysql;
    $mr = mysql_query($s, $mysql);

    if (!$mr)
      die("ERROR: Bad query; ".mysql_error());

    $num_rows = mysql_num_rows($mr);

    $ret = array();

    while($row = mysql_fetch_array($mr))
      array_push($ret, $row);

    return $num_rows;
  } // fetchRowBySqlStatement

  function fetchRow($s, &$ret) {
    global $mysql;
    $mr = mysql_query($s, $mysql);
    if (!$mr)
      die("ERROR: Bad query; ".mysql_error());
    $num_rows = mysql_num_rows($mr);

    while($row = mysql_fetch_array($mr)) {
      $ret = $row;
      break;
    } // while

    return $num_rows;
  } // fetchRow

  function update($sqls) {
    global $mysql;

    $ret = mysql_query($sqls, $mysql);

    if (!$ret)
      die("ERROR: Bad query; ".mysql_error());

    return mysql_affected_rows($mysql);
  } // insert

?>
