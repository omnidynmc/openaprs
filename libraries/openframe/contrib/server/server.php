#!/usr/bin/php
<?

  $sock = stream_socket_server("tcp://0.0.0.0:10152", $errno, $errstr);

  if (!$sock) {
    die("$errstr ($errno)");
  } // if

  $stats_interval = 5;
  $max_pps = 1000;
  $peers = array();

  while(true) {
    $con = @stream_socket_accept($sock, 0.01, $peername);
    if ($con) {
      $peers[$peername]['connected_at'] = time();
      $peers[$peername]['last_stats_at'] = time();
      $peers[$peername]['sock'] = $con;
      $peers[$peername]['peername'] = $peername;
      $peers[$peername]['num_packets'] = 0;
      $peers[$peername]['num_bytes'] = 0;
      echo "Connected to $peername\n";
    } // if

    $remove_peers = array();
    foreach($peers AS &$peer) {
      $peername = $peer['peername'];
      for($i=0; $i < 100; $i++) {
        $buf = "This is a test\n";
        $ret = @fwrite($peer['sock'], "This is a test\n");
        if ($ret < 1) {
          echo "Disconnected from $peer[peername]\n";
          array_push($remove_peers, $peername);
          fclose($peer['sock']);
          break;
        } // if

        $peer['num_packets']++;
        $peer['num_bytes'] += $ret;

        // we want 30 pps so take number of clients, 1000000 microseconds in a second
        $usleep_time = ceil( count($peers) ? 1000000 / (count($peers) * $max_pps) : 10000 );
        usleep($usleep_time);
      } // for

//echo $peer['last_stats_at'] . " " . (time() - $stats_interval) . "\n";
      if ($peer['last_stats_at'] < time() - $stats_interval) {
        $diff = time() - $peer['last_stats_at'];
        $pps = $peer['num_packets'] / $diff;
        $bps = $peer['num_bytes'] / $diff;
        $usleep_time = ceil( count($peers) ? 1000000 / (count($peers) * $max_pps) : 10000 );
        echo "Stats " . $peer['peername']
             . ", pps " . number_format($pps, 2)
             . ", bps " . number_format($bps, 2)
             . ", peers " . count($peers)
             . ", usleep " . intval($usleep_time)
             . "\n";

        $peer['num_packets'] = 0;
        $peer['num_bytes'] = 0;
        $peer['last_stats_at'] = time();
      } // if
    } // for

    foreach($remove_peers AS $peername) {
      unset($peers[$peername]);
    } // foreach

    $usleep_time = ceil( count($peers) ? 1000000 / (count($peers) * $max_pps) : 10000 );
    usleep($usleep_time);
  } // while

  fclose($sock);
?>
