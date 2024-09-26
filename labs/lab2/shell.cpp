/****************
LE2: Basic Shell
****************/
#include <unistd.h>     // pipe, fork, dup2, execvp, close
#include <sys/wait.h>   // wait
#include <iostream>     // io operations
#include "Tokenizer.h"

#include <readline/readline.h>
#include <readline/history.h>

/* Used for select(2) */
#include <sys/types.h>
#include <sys/select.h>

using namespace std;


enum IO {READ, WRITE};    // read and write ends of a pipe

void execute_command (Command* command, const int* pipefd, const bool isLast)
{
    char** args = command->argsToCString();

    if (!isLast) {              // Redirect the output, if we arent on the last command
        dup2(pipefd[IO::WRITE], STDOUT_FILENO);
    }
    close(pipefd[IO::READ]);    // Close the read end of the pipe on the child side.
    execvp(args[0], args);      // In child, execute the command
    
    delete[] args;
}

string get_expansion_result() {
    char buffer[1000];
    size_t size = read(STDIN_FILENO, buffer, 1000);
    if ((size > 0) && (size < sizeof(buffer))) {
        buffer[size]='\0';
        return string{buffer};
    } else {
        perror("pipe_read");
        exit(-1);
    }
}

void execute_line (string& command_line)
{
    Tokenizer line{command_line};

    // // print out every command token-by-token on individual lines
    // // prints to cerr to avoid influencing autograder
    // for (auto cmd : line.commands) {
    //     cerr << "\t\033[1;36m" << "cmd:";
    //     for (auto str : cmd->args) {
    //         cerr << "\t[" << "\033[1;33m" << str << "\033[1;36m" << "] ";
    //     }
    //     if (cmd->hasInput()) {
    //         cerr << "in< " << cmd->in_file << " ";
    //     }
    //     if (cmd->hasOutput()) {
    //         cerr << "out> " << cmd->out_file << " ";
    //     }
    //     cerr << "\033[0m" << endl;
    // }

    // vector<string> se_results;
    // for (auto it = line.inner_sign_expansions.begin(); it != line.inner_sign_expansions.end(); ++it) {
    //     Tokenizer curr{*it, line.inner_strings};
    //     if (curr.hasError()) return;
    //     // exec_line(curr);
    //     // se_results.push_back(get_expansion_result());
    //     se_results.push_back("\033[1;31m[" + *it + "]\033[1;33m");
        
    // }
    // for (auto it = line.se_locations.begin(); it != line.se_locations.end(); ++it) {
    //     line.commands.at(*it)->signExpand(se_results);
    // }

    // for (auto cmd : line.commands) {
    //     cerr << "\t\033[1;36m" << "cmd:";
    //     for (auto str : cmd->args) {
    //         cerr << "\t[" << "\033[1;33m" << str << "\033[1;36m" << "] ";
    //     }
    //     if (cmd->hasInput()) {
    //         cerr << "in< " << cmd->in_file << " ";
    //     }
    //     if (cmd->hasOutput()) {
    //         cerr << "out> " << cmd->out_file << " ";
    //     }
    //     cerr << "\033[0m" << endl;
    // }

    // return;

    for (Command*& command : line.commands)
    {
        bool isLast = &command == &line.commands.back();

        int pipefd[2];
        pipe(pipefd); // Create pipe, use for shell<->child communication        
        
        int childpid = fork();

        if (childpid == 0) {      // Child side
            execute_command(command, pipefd, isLast);
            return;
        }
        else {                // Parent side
            dup2(pipefd[IO::READ], STDIN_FILENO); // Redirect the Shell's input to the read end of the pipe.
            close(pipefd[IO::WRITE]);  // Close the write end of the pipe
            
            // Wait until last command finishes
            if (isLast) {
                //waitpid(chik0);
                waitpid(childpid, NULL, 0);
            }
        }
    }
}

int main()
{
    cout << "Welcome to the \"Dumb Ugly Mock-Bash\" Shell, AKA the D.U.M.B. Shell.\n" \
        "Made by Cole McAnelly for a computer science lab\n" \
        "Type \"Exit\", \"quit\", or just \"q\" to quit\n" << endl;
    
    int originalIO[2] = { dup(STDIN_FILENO), dup(STDOUT_FILENO) };

    while (true)
    {
        cout << "(dumb) ";
        string command_line;
        getline(cin, command_line);

        if (command_line == "exit" || command_line == "q" || command_line == "Exit" || command_line == "quit") {
            return 0;
        }

        execute_line(command_line);

        // Restore the stdin and stdout
        dup2(originalIO[IO::READ], STDIN_FILENO);
        dup2(originalIO[IO::WRITE], STDOUT_FILENO);
    }
}