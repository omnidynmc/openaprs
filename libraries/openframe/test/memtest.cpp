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

class TestClass : public openframe::counter<TestClass> {
  public:
    TestClass() { }
    virtual ~TestClass() { }

  protected:
  private:
}; // class TestClass

void test() {
  std::vector<TestClass *> *v = new std::vector<TestClass *>;

  for(size_t i=0; i < 5000000; i++) {
    TestClass *tc = new TestClass;
    v->push_back(tc);
  } // for

  std::cout << "check mem" << std::endl;
  sleep(10);

  for(size_t i=0; i < 5000000; i++) {
    TestClass *tc = v->at(i);
    delete tc;
  } // for

  std::cout << "check mem" << std::endl;
  sleep(10);

  v->clear();

  std::cout << "check mem" << std::endl;
  sleep(10);

  delete v;

  std::cout << "done, check mem" << std::endl;
} // test

int main(int argc, char **argv) {

  test();

  while(true) sleep(1);

  exit(0);
} // main
