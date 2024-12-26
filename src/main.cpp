#include <iostream>

int main() {
  // Flush after every std::cout / std:cerr
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;
  while (true)
  {
    // Uncomment this block to pass the first stage
    std::cout << "$ ";

    std::string input;
    std::getline(std::cin, input);
    if(input=="exit 0") {
      return 0;
    } else if(input.find("echo ")==0) {
      std::string text = input.substr(5);
      std::cout << text << "\n";
    } else if(input.find("type ")==0) {
      std::string text = input.substr(5);
      if(text.substr(0,4) == "type" || text.substr(0,4) == "exit" || text.substr(0,4) == "echo") {
        std::cout << text << " is a shell builtin\n";
      } else {
        std::cout << text << " not found\n";
      }
    } else {
      std::cout << input << ": command not found\n";
    }
  }
}
