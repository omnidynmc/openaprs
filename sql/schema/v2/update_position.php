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
  $prev = $day + 1;

  while(1) {
    $day--;

    if ($day == 0)
      break;

    echo "DAY($day) PREV($prev)\n";
    $prev = $day + 1;
    while(1) {
      $sqls = "SELECT path, packet_date, callsign_id FROM position WHERE path_id=0 AND packet_date <= DATE(NOW()) - INTERVAL $day DAY AND packet_date >= DATE(NOW()) - INTERVAL $prev DAY LIMIT 1000";
      $numRows = fetch($sqls, $ret);
      if ($numRows < 1) {
        echo "No rows found.\n";
        break;
      } # if

      echo "Selected " . $numRows . " rows\n";

      foreach($ret AS $row) {
        $hash = md5($row[path]);
        $body = mysql_real_escape_string($row[path]);
        $sqls ="SELECT id FROM path WHERE hash='$hash'";
        $num = fetchRow($sqls, $path);
        $id = $path[id];
        if ($num < 1) {
          $sqls ="INSERT IGNORE INTO path (hash, body, create_ts) VALUES ('$hash', '$body', UNIX_TIMESTAMP())";
          $num = insert($sqls, $id);
          if ($num < 1 || $id == 0) {
            echo "ERROR: Could not insert path $hash, $body";
            continue;
          } // if
          echo "Inserted id " . $id . " for " . $hash . "\n";
        } // if

        echo "Found id " . $id . " for " . $hash . "\n";
        $sqls = "UPDATE position SET path_id='$id' WHERE packet_date='$row[packet_date]' AND callsign_id='$row[callsign_id]' AND path_id=0 LIMIT 1";
        $num = update($sqls);
        echo "Updated " . $num . " rows for " . $row[callsign_id] . "\n";
      } // foreach
    } // while
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

  function insert($sqls, &$id) {
    global $mysql;

    $ret = mysql_query($sqls, $mysql);

    if (!$ret)
      die("ERROR: Bad query; ".mysql_error());

    $numAffected = mysql_affected_rows($mysql);

    if ($numAffected)
      $id = mysql_insert_id($mysql);

    return $numAffected;
  } // insert

?>
