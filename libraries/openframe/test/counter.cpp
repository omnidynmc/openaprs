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

int main(int argc, char **argv) {
  std::vector<TestClass *> v;

  for(size_t i=0; i < 1024; i++) {
    TestClass *tc = new TestClass;
    v.push_back(tc);
  } // for

  for(size_t i=0; i < 100; i++) {
    delete v[i];
  } // for

  TestClass tc;
  std::cout << "created=" << tc.objects_created
            << ",alive=" << tc.objects_alive
            << std::endl;

  exit(0);
} // main
