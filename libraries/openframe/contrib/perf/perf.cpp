#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <iomanip>

#include <time.h>

int main(int argc, char **argv) {
  const time_t intval = 15;
  time_t last_ts = time(NULL);
  size_t loop_count = 0;

  for(size_t i=0; i < 100000; i++) {
    if (last_ts < time(NULL) - intval) {
      time_t diff = time(NULL) - last_ts;
      double lpf = double(loop_count) / double(diff);
      std::cout << std::fixed << std::setprecision(3) << "lpf=" << lpf << std::endl;
      last_ts = time(NULL);
      loop_count = 0;
    } // if

    loop_count++;
    usleep(100);
  } // while
} // main
