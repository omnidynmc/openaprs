#include <cassert>
#include <exception>
#include <iostream>
#include <new>
#include <string>

#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <openframe/openframe.h>
#include <openframe/counter.h>

int main(int argc, char **argv) {

 openframe::Logger elog;
  elog.open("app", "loghup.log");
  elog.level("app", openframe::loglevel::LogDebug);

  for(size_t i = 0; i < 10; i++) {
    elog["app"] << openframe::loglevel::LogDebug << "1) This is a log test " << i << std::endl;
    elog["app"] << openframe::loglevel::LogDebug << "2) This is a log test " << i << std::endl;
    elog["app"] << openframe::loglevel::LogDebug << "3) This is a log test " << i << std::endl;
    elog["app"] << openframe::loglevel::LogDebug << "4) This is a log test " << i << std::endl;
    elog["app"] << openframe::loglevel::LogDebug << "5) This is a log test " << i << std::endl;
    elog.hup();
    elog["app"] << openframe::loglevel::LogDebug << "6) This is a log test " << i << std::endl;
    elog["app"] << openframe::loglevel::LogDebug << "7) This is a log test " << i << std::endl;
    elog["app"] << openframe::loglevel::LogDebug << "8) This is a log test " << i << std::endl;
    elog["app"] << openframe::loglevel::LogDebug << "9) This is a log test " << i << std::endl;
    elog["app"] << openframe::loglevel::LogDebug << "10) This is a log test " << i << std::endl;
    sleep(1);
  } // for

  exit(0);
} // main
