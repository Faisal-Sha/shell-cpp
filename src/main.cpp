#include <iostream>
#include <string>
#include <sstream>
#include <filesystem>


std::string get_path(std::string cmd) {
  std::string path_env = std::getenv("PATH");

  std::stringstream ss(path_env);
  std::string path;

  while (!ss.eof())
  {
    getline(ss, path, ':');
    std::string abs_path = path + "/" + cmd;
    if (std::filesystem::exists(abs_path))
    {
      return abs_path;
    }
  }
  
  return "";
}

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
        std::string path = get_path(text);
        if(path.empty()) {
          std::cout << text << ": not found\n";
        } else {
          std::cout<< text << " is " << path << "\n";
        }
      }
    } else {
      std::cout << input << ": command not found\n";
    }
  }
}
