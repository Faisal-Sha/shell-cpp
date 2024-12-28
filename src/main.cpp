#include <iostream>
#include <string>
#include <sstream>
#include <filesystem>
#include <vector>
#include <sys/wait.h>
#include <fstream> // For file reading

using namespace std;

// Function to split a string into tokens
vector<string> split(const string &input) {
  vector<string> tokens;
  size_t i = 0;
  while (i < input.length())
  {
    if (isspace(input[i]))
    {
      ++i; // Skip whitespace
      continue;
    }
    string token;
    if (input[i] == '\'')
    {
      // Handle single quotes
      ++i; // Skip opening quote
      while (i < input.length() && input[i] != '\'')
      {
        token += input[i++];
      }
      if (i == input.length() || input[i] != '\'')
      {
        cerr << "Error: unmatched single quote\n";
        return {};
      }
      ++i; // Skip closing quote
    }
    else if (input[i] == '"')
    {
      // Handle double quotes
      ++i; // Skip opening quote
      while (i < input.length() && input[i] != '"')
      {
        if (input[i] == '\\' && i + 1 < input.length() &&
            (input[i + 1] == '\\' || input[i + 1] == '"' || input[i + 1] == '$' || input[i + 1] == '\n'))
        {
          token += input[++i]; // Add escaped character
        }
        else
        {
          token += input[i];
        }
        ++i;
      }
      if (i == input.length() || input[i] != '"')
      {
        cerr << "Error: unmatched double quote\n";
        return {};
      }
      ++i; // Skip closing quote
    }
    else
    {
      // Handle unquoted tokens and backslashes
      while (i < input.length() && !isspace(input[i]))
      {
        if (input[i] == '\\' && i + 1 < input.length())
        {
          token += input[++i]; // Add escaped character
        }
        else
        {
          token += input[i];
        }
        ++i;
      }
    }
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
      if (tokens.size() > 1 && (tokens[1] == "echo" || tokens[1] == "exit" || tokens[1] == "type" || tokens[1] == "pwd")) {
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
      string cwd = filesystem::current_path();
      string res = cwd.substr(0, cwd.length());
      cout << res << "\n";
      continue;
    } else if (tokens[0] == "cd") {
      if (tokens.size() > 1) {
        string path = tokens[1];

        // Handle '~' for home directory
        if (path[0] == '~') {
          const char* home_dir = getenv("HOME");
          if (home_dir) {
            path.replace(0, 1, home_dir);  // Replace ~ with the home directory path
          } else {
            cerr << "cd: HOME not set\n";
            continue;
          }
        }

        // Handle absolute paths
        if (path[0] == '/') {
          if (filesystem::exists(path) && filesystem::is_directory(path)) {
            filesystem::current_path(path); // Change the current directory
          } else {
            cerr << "cd: " << path << ": No such file or directory\n";
          }
        } else {
          // Handle relative paths
          try {
            // Get the current working directory
            string current_dir = filesystem::current_path();
            
            // Combine it with the relative path to get the full path
            filesystem::path full_path = filesystem::path(current_dir) / path;
            
            // Check if the resolved path exists and is a directory
            if (filesystem::exists(full_path) && filesystem::is_directory(full_path)) {
              filesystem::current_path(full_path); // Change the current directory
            } else {
              cerr << "cd: " << full_path << ": No such file or directory\n";
            }
          } catch (const filesystem::filesystem_error &e) {
            cerr << "cd: " << e.what() << "\n";
          }
        }
      } else {
        cerr << "cd: missing operand\n";
      }
      continue;
    } else if (tokens[0] == "cat") {
      for (size_t i = 1; i < tokens.size(); ++i) {
          ifstream file(tokens[i]);
          if (file) {
            cout << file.rdbuf(); // Output file content
          } else {
            cerr << "cat: " << tokens[i] << ": No such file or directory\n";
          }
      }
      continue;
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
