#include "include/repo.hpp"

#include <iostream>
#include <string>

int main(int argc, char **argv) {
  if (argc < 2) {
    std::cout << "Usage: dagr <command>\n";
    return 1;
  }

  std::string command = argv[1];

  if (command == "init") {
    Repo::init();
  } else {
    std::cout << "Unknown command\n";
  }

  return 0;
}
