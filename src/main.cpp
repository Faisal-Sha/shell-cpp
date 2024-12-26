#include <iostream>
#include <string>
#include <sstream>
#include <filesystem>
#include <vector>
#include <sys/wait.h>

using namespace std;

// Function to split a string into tokens
vector<string> split(const string &str) {
    vector<string> tokens;
    stringstream ss(str);
    string token;
    while (ss >> token) {
        tokens.push_back(token);
    }
    return tokens;
};

string get_path(string cmd) {
  string path_env = getenv("PATH");

  stringstream ss(path_env);
  string path;

  while (!ss.eof())
  {
    getline(ss, path, ':');
    string abs_path = path + "/" + cmd;
    if (filesystem::exists(abs_path))
    {
      return abs_path;
    }
  }
  
  return "";
}

int main() {
  // Flush after every cout / std:cerr
  cout << unitbuf;
  cerr << unitbuf;
  while (true) {
    // Prompt
    cout << "$ ";

    // Get input
    string input;
    getline(cin, input);

    // Exit on "exit 0"
    if (input == "exit 0") {
        return 0;
    }

    // Split the input into tokens
    vector<string> tokens = split(input);
    if (tokens.empty()) {
      continue; // Skip empty input
    }
    
    // Handle built-in commands
    if (tokens[0] == "echo") {
      for (size_t i = 1; i < tokens.size(); ++i) {
        cout << tokens[i] << (i == tokens.size() - 1 ? "\n" : " ");
      }
      continue;
    } else if (tokens[0] == "type") {
      if (tokens.size() > 1 && (tokens[1] == "echo" || tokens[1] == "exit" || tokens[1] == "type")) {
        cout << tokens[1] << " is a shell builtin\n";
      } else if (tokens.size() > 1) {
        string path = get_path(tokens[1]);
        if (path.empty()) {
          cout << tokens[1] << ": not found\n";
        } else {
          cout << tokens[1] << " is " << path << "\n";
        }
      } else {
        cerr << "type: missing operand\n";
      }
      continue;
    } else if(tokens[0] == "pwd") {
      // Get and print the current working directory
      cout << filesystem::current_path() << "\n";
    }

    // Handle external commands
    string program_path = get_path(tokens[0]);
    if (program_path.empty()) {
      cout << tokens[0] << ": command not found\n";
      continue;
    }

    // Fork and execute the external command
    pid_t pid = fork();
    if (pid == 0) {
      // Child process
      vector<char *> args;
      for (const auto &token : tokens) {
        args.push_back(const_cast<char *>(token.c_str()));
      }
      args.push_back(nullptr);

      // Replace the current process image with the program
      execvp(program_path.c_str(), args.data());
      // If execvp fails
      cerr << "Error: Failed to execute " << program_path << "\n";
      exit(1);
    } else if (pid > 0) {
      // Parent process
      int status;
      waitpid(pid, &status, 0); // Wait for the child process to finish
    } else {
      // Fork failed
      cerr << "Error: Failed to fork process\n";
    }
  }
}
