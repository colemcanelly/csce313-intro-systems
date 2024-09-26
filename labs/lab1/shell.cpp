/****************
LE1: Introduction to Unnamed Pipes
****************/
#include <unistd.h> // pipe, fork, dup2, execvp, close
using namespace std;

int main () {
    // lists all the files in the root directory in the long format
    char* cmd1[] = {(char*) "ls", (char*) "-al", (char*) "/", nullptr};
    // translates all input from lowercase to uppercase
    char* cmd2[] = {(char*) "tr", (char*) "a-z", (char*) "A-Z", nullptr};

    // TODO: add functionality
    int pipefd[2];
    pipe(pipefd);   // Create pipe, pass file descriptor; 0 = read, 1 = write.
    
    const __uint8_t READ = 0;               // Read-end of pipe
    const __uint8_t WRITE = 1;              // Write-end of pipe

    /* CHILD 1 */
    int pid1 = fork();                      // Create child to run first command
    if (pid1 == 0) // IF: child
    {                                       // In child 1
        dup2(pipefd[WRITE], STDOUT_FILENO); // redirect output to write end of pipe
        close(pipefd[READ]);                // Close the read end of the pipe on the child side.
        execvp(cmd1[0], cmd1);              // In child, execute the command
    }

    /* CHILD 2 */
    int pid2 = fork();                      // Create another child to run second command
    if (pid2 == 0) // IF: child
    {                                       // In child 2
        dup2(pipefd[READ], STDIN_FILENO);   // redirect input to the read end of the pipe
        close(pipefd[WRITE]);               // Close the write end of the pipe on the child side
        execvp(cmd2[0], cmd2);              // Execute the second command.
    }

    // Reset the input and output file descriptors of the parent.
    dup2(pipefd[READ], STDIN_FILENO);
    dup2(pipefd[WRITE], STDOUT_FILENO);
}