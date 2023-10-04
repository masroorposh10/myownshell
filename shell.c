#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "parser.h"
#define BUFLEN 1024
#define MAX_ARGS 10



int main() {
    char buffer[BUFLEN];
    char *parsedinput;
    char *args[MAX_ARGS + 1]; // +1 for NULL terminator

    printf("Welcome to the Group04 shell! Enter commands, enter 'quit' to exit\n");
    
    do {
        // Print the terminal prompt and get input
        printf("$ ");
        char *input = fgets(buffer, sizeof(buffer), stdin);
        if (!input) {
            fprintf(stderr, "Error reading input\n");
            return -1;
        }

        // Clean and parse the input string
        parsedinput = (char *)malloc(BUFLEN * sizeof(char));
        size_t parselength = trimstring(parsedinput, input, BUFLEN);

        // Sample shell logic implementation
        if (strcmp(parsedinput, "quit") == 0) {
            printf("Bye!!\n");
            free(parsedinput);
            return 0;
        }

        // Check for background execution
        int background = 0;
        if (parsedinput[parselength - 1] == '&') {
            background = 1;
            // Remove the '&' from the input
            parsedinput[parselength - 1] = '\0';
        }

        // Tokenize the input and populate the args array
        char *token = strtok(parsedinput, " "); // Assuming space as the delimiter
        int argCount = 0;
        while (token != NULL && argCount < MAX_ARGS) {
            args[argCount++] = token;
            token = strtok(NULL, " ");
        }
        args[argCount] = NULL; // Null-terminate the args array

        // Check for the pipe operator '|'
        int pipe_index = -1;
        for (int i = 0; i < argCount; i++) {
            if (strcmp(args[i], "|") == 0) {
                pipe_index = i;
                break;
            }
        }

        if (pipe_index != -1) {
            // Execute a command with a pipe
            char *command1[MAX_ARGS + 1];
            char *command2[MAX_ARGS + 1];

            for (int i = 0; i < pipe_index; i++)
                command1[i] = args[i];
            command1[pipe_index] = NULL;

            int j = 0;
            for (int i = pipe_index + 1; i < argCount; i++)
                command2[j++] = args[i];
            command2[j] = NULL;

            int pipefd[2];
            if (pipe(pipefd) == -1) {
                perror("Pipe creation failed");
                return -1;
            }

            pid_t forkV1 = fork();
            if (forkV1 == 0) {
                // Child 1: Execute the first command and write to the pipe
                close(pipefd[0]);
                dup2(pipefd[1], STDOUT_FILENO);
                close(pipefd[1]);

                execute_command(command1, 0); // Not background
            } else {
                close(pipefd[1]);

                pid_t forkV2 = fork();
                if (forkV2 == 0) {
                    // Child 2: Read from the pipe and execute the second command
                    dup2(pipefd[0], STDIN_FILENO);
                    close(pipefd[0]);

                    execute_command(command2, 0); // Not background
                } else {
                    close(pipefd[0]);
                    wait(NULL); // Wait for child 2 to finish
                }

                wait(NULL); // Wait for child 1 to finish
            }
        } else {
            // No pipe, execute a single command
            execute_command(args, background);
        }

        // Print background command termination messages
        if (background) {
            int status;
            pid_t terminated_pid;
            do {
                terminated_pid = waitpid(-1, &status, WNOHANG);
                if (terminated_pid > 0) {
                    printf("Background command %d terminated\n", terminated_pid);
                }
            } while (terminated_pid > 0);
        }

        // Remember to free any memory you allocate!
        free(parsedinput);
    } while (1);

    return 0;
}
