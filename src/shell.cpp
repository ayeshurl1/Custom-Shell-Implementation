\
    // shell.cpp - Simple custom shell with single-pipe support
    #include <iostream>
    #include <sstream>
    #include <vector>
    #include <unistd.h>
    #include <sys/wait.h>
    #include <cstring>

    using namespace std;

    vector<string> split(const string &s){
      vector<string> out; istringstream iss(s); string cur;
      while (iss >> cur) out.push_back(cur);
      return out;
    }

    int main(){
      string line;
      while (true){
        cout << "myshell> ";
        if (!getline(cin, line)) break;
        if (line.empty()) continue;
        if (line == "exit") break;
        size_t pipe_pos = line.find('|');
        if (pipe_pos == string::npos){
          auto args = split(line);
          if (args.empty()) continue;
          vector<char*> argv;
          for (auto &a: args) argv.push_back(strdup(a.c_str()));
          argv.push_back(nullptr);
          pid_t pid = fork();
          if (pid == 0){
            execvp(argv[0], argv.data());
            perror("exec");
            exit(1);
          } else {
            waitpid(pid, nullptr, 0);
          }
        } else {
          string left = line.substr(0, pipe_pos);
          string right = line.substr(pipe_pos + 1);
          int fd[2];
          if (pipe(fd) == -1) { perror("pipe"); continue; }
          pid_t p1 = fork();
          if (p1 == 0){
            dup2(fd[1], STDOUT_FILENO);
            close(fd[0]); close(fd[1]);
            auto args = split(left);
            vector<char*> argv;
            for (auto &a: args) argv.push_back(strdup(a.c_str()));
            argv.push_back(nullptr);
            execvp(argv[0], argv.data());
            perror("exec1"); exit(1);
          }
          pid_t p2 = fork();
          if (p2 == 0){
            dup2(fd[0], STDIN_FILENO);
            close(fd[0]); close(fd[1]);
            auto args = split(right);
            vector<char*> argv;
            for (auto &a: args) argv.push_back(strdup(a.c_str()));
            argv.push_back(nullptr);
            execvp(argv[0], argv.data());
            perror("exec2"); exit(1);
          }
          close(fd[0]); close(fd[1]);
          waitpid(p1, nullptr, 0);
          waitpid(p2, nullptr, 0);
        }
      }
      return 0;
    }
