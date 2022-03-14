#!/usr/bin/php
<?

  $stats_interval = 5;
  $max_pps = 1000;
  $connect_to = "tcp://localhost:10152";
  $fail_sleep = 15;

  while(true) {
    echo "Trying to connect to $connect_to\n";
    $con = @stream_socket_client($connect_to, $errno, $errstr);
    if (!$con) {
      echo "Error \"$errstr\" connecting to $connect_to\n";
      sleepfor($fail_sleep);
    } // if

    $peer['sock'] = $con;
    $peer['peername'] = $connect_to;
    $peer['last_stats_at'] = time();
    $peer['num_packets'] = 0;
    $peer['num_bytes'] = 0;

    echo "Connected to $connect_to\n";
    while(true) {
      $buf = "This is a test\n";
      $ret = @fwrite($peer['sock'], "This is a test\n");
      if ($ret < 1) {
        echo "Disconnected from $peer[peername]\n";
        fclose($peer['sock']);
        unset($peer);
        break;
      } // if

      $peer['num_packets']++;
      $peer['num_bytes'] += $ret;

      if ($peer['last_stats_at'] < time() - $stats_interval) {
        $diff = time() - $peer['last_stats_at'];
        $pps = $peer['num_packets'] / $diff;
        $bps = $peer['num_bytes'] / $diff;
        $usleep_time = 1000000 / $max_pps;
        echo "Stats " . $peer['peername']
             . ", pps " . number_format($pps, 2)
             . ", bps " . number_format($bps, 2)
             . ", usleep " . intval($usleep_time)
             . "\n";

        $peer['num_packets'] = 0;
        $peer['num_bytes'] = 0;
        $peer['last_stats_at'] = time();
      } // if

      // we want 30 pps so take number of clients, 1000000 microseconds in a second
      $usleep_time = 1000000 / $max_pps;
      usleep($usleep_time);
    } // while

    sleepfor(15);
  } // while

  function sleepfor($seconds) {
    for($i=0; $i < $seconds; $i++) {
      if ($i % 5 == 0) echo "Sleeping for " . ($seconds - $i) . " seconds\n";
      sleep(1);
    } // for
  } // sleepfor
?>
