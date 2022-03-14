#!/usr/php
<?
  $last_ts = time();
  $interval = 15;
  $loop_count = 0;
  while(1) {
    if ($last_ts < time()-$interval) {
      $diff = time() - $last_ts;
      $lps = number_format($loop_count / $diff, 3);
      echo "lps=" . $lps . "\n";
      $last_ts = time();
      $loop_count = 0;
    } // if
    $loop_count++;
    usleep(1000);
  } // while
?>
