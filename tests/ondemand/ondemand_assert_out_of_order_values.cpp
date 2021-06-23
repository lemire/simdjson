#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include "simdjson.h"

using namespace simdjson;

// This ensures the compiler can't rearrange them into the proper order (which causes it to work!)
simdjson_never_inline bool check_point(simdjson_result<ondemand::value> xval, simdjson_result<ondemand::value> yval) {
  // Verify the expected release behavior
  uint64_t x, y;
  if (!xval.get(x)) { return false; }
  if (!yval.get(y)) { return false; }
  std::cout << x << "," << y << std::endl;
  return true;
}

bool test_check_point() {
  auto json = std::string_view(R"(
    {
      "x": 1,
      "y": 2 3
  )");
  ondemand::parser parser;
  auto doc = parser.iterate(json);
  return check_point(doc["x"], doc["y"]);
}

// Run a test function in a fork and check whether it exited with an assert signal
template<typename F>
bool assert_test(const F& f) {
  pid_t pid = fork();
  if (pid == -1) {
    std::cerr << "fork failed" << std::endl;
    exit(1);
  }

  if (!pid) {
    //
    // This code runs in the fork (so we run the test function)
    //
    bool succeeded = f();
    exit(succeeded ? 0 : 1);
  }

  //
  // This code runs in the original executable (so we wait for the fork and check the exit code)
  //
  int exit_code = 0;
  if (waitpid(pid, &exit_code, 0) == pid_t(-1)) {
    std::cerr << "waitpid failed: " << std::string_view(strerror(errno)) << std::endl;
    exit(1);
  }

  // Test passes if the child exited with an assert signal
  return WIFSIGNALED(exit_code);
}

int main(void) {

  // To verify that the program asserts (which sends a signal), we fork a new process and use wait()
  // to check the resulting error code. If it's a signal error code, we consider the
  // test to have passed.
  // From https://stackoverflow.com/a/33694733

  bool succeeded = true;
#ifndef NDEBUG
  succeeded |= assert_test(test_check_point);
#endif
  return succeeded ? 0 : 1;
}
