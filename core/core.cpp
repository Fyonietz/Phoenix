
#include "engine.hpp"
#include <iostream>

int main(void) {

  engine_start();

  // Wait for user input to stop the server
  getchar();

  // Stop the server
  mg_stop(Config::context);
  return 0;
}
