#include "parser.h"
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
size_t trimstring(char* outputbuffer, const char* inputbuffer, size_t bufferlen)
{   
    memcpy(outputbuffer, inputbuffer, bufferlen * sizeof(char));

    for (size_t ii = strlen(outputbuffer) - 1; ii >= 0; ii--) {
        if (outputbuffer[ii] < '!') // In ASCII '!' is the first printable (non-control) character
        {
            outputbuffer[ii] = 0;
        } else {
            break;
        }    
    }

    return strlen(outputbuffer);
}

size_t firstword(char* outputbuffer, const char* inputbuffer, size_t bufferlen)
{
    // Find the first word in the input string
    size_t i = 0;
    while (i < bufferlen && inputbuffer[i] != ' ' && inputbuffer[i] != '\0' && inputbuffer[i] != '\n') {
        outputbuffer[i] = inputbuffer[i];
        i++;
    }
    outputbuffer[i] = '\0';
    
    return strlen(outputbuffer);
}

bool isvalidascii(const char* inputbuffer, size_t bufferlen)
{
    // Check if the input string contains only valid ASCII characters
    for (size_t i = 0; i < bufferlen; i++) {
        if ((unsigned char)inputbuffer[i] < ' ' || (unsigned char)inputbuffer[i] > '~') {
            return false;
        }
    }
    return true;
}

int findpipe(const char* inputbuffer, size_t bufferlen)
{
    // Find the location of the pipe character in the input string
    for (size_t i = 0; i < bufferlen; i++) {
        if (inputbuffer[i] == '|') {
            return i;
        }
    }
    return -1;
}

bool runinbackground(const char* inputbuffer, size_t bufferlen)
{
    // Check if the input string ends with "&"
    if (bufferlen >= 2 && inputbuffer[bufferlen - 2] == '&' && inputbuffer[bufferlen - 1] == '\n') {
        return true;
    }
    return false;
}
void execute_command(char* args[], int background) {
    pid_t pid = fork();

    if (pid == -1) {
        perror("Fork failed");
    } else if (pid == 0) {
        // Child process
        if (background) {
            // If it's a background command, detach the child from the terminal
            setsid();
        }

        if (execvp(args[0], args) == -1) {
            perror("Command execution failed");
            exit(1);
        }
    } else {
        // Parent process
        if (!background) {
            // Wait for the child to finish, but only if it's not a background command
            int status;
            waitpid(pid, &status, 0);
        }
    }
}