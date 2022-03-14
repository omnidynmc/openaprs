#!/usr/bin/php
<?php
  $cfg['mysql_host'] = "localhost";
  $cfg['mysql_user'] = "root";
  $cfg['mysql_pass'] = "";
  $cfg['mysql_db'] = "openaprs";

  define("SQL_FILE", "slow_lastposition.sql");

  $mysql = mysql_connect($cfg['mysql_host'], $cfg['mysql_user'], $cfg['mysql_pass']);

  if (!$mysql)
    die("ERROR: Unable to connect to mysql; ".mysql_error());

  if (!mysql_select_db($cfg['mysql_db']))
    die("ERROR: Unable to use db; ".mysql_error());

  $buf = file_get_contents(SQL_FILE);

  $line = preg_split("/\n/", $buf);

  while(true) {
    $total = 0;
    reset($line);
    $newline = array();
    foreach($line AS $sqls) {
      if (!strlen($sqls))
        continue;
      $numRows = update($sqls);
      $total += $numRows;
      echo $numRows . " - " . substr($sqls, 0, 20) . "\n";

      if ($numRows)
        array_push($newline, $sqls);
    } // foreach

    $line = $newline;

    if (!$total)
      break;
  } // while

  mysql_close($mysql);

  function update($sqls) {
    global $mysql;

    $ret = mysql_query($sqls, $mysql);

    if (!$ret)
      die("ERROR: Bad query; ".mysql_error());

    return mysql_affected_rows($mysql);
  } // insert

?>
