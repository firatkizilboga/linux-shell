#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    int pipefd[2];
    pid_t pid;
    ssize_t bytes_read;
    const size_t bufferSize = 1024; // Define your buffer size here

    if (pipe(pipefd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    pid = fork();
    if (pid < 0) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {
        // Child process
        close(pipefd[0]); // Close unused read end
        dup2(pipefd[1], STDOUT_FILENO); // Redirect stdout to pipe
        close(pipefd[1]); // Close write end of the pipe

        char *args[] = {"cat", "example.txt", NULL}; // Replace 'example.txt' with your file
        execvp("cat", args);
        perror("execvp");
        exit(EXIT_FAILURE);
    } else {
        // Parent process
        close(pipefd[1]); // Close unused write end

        char *buffer = malloc(bufferSize); // Dynamically allocate memory
        if (!buffer) {
            perror("malloc");
            exit(EXIT_FAILURE);
        }

        while ((bytes_read = read(pipefd[0], buffer, bufferSize - 1)) > 0) {
            buffer[bytes_read] = '\0'; // Null terminate the string
            printf("%s", buffer);
        }

        free(buffer); // Free the allocated memory
        close(pipefd[0]); // Close read end of the pipe
        wait(NULL); // Wait for child to finish
    }

    return 0;
}
