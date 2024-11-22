#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>

#define BUFFER_SIZE 1024

// Function to reverse the case of each character
void reverse_case(char *message) {
    for (int i = 0; message[i] != '\0'; i++) {
        if (islower(message[i])) {
            message[i] = toupper(message[i]);
        } else if (isupper(message[i])) {
            message[i] = tolower(message[i]);
        }
    }
}

int main() {
    int pipefd[2];
    pid_t pid;
    char buffer[BUFFER_SIZE];

    // Create the pipe
    if (pipe(pipefd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    // Create the child process
    pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {
        // Child process
        close(pipefd[1]); // Close the write end of the pipe

        // Read message from the pipe
        ssize_t bytes_read = read(pipefd[0], buffer, BUFFER_SIZE);
        if (bytes_read == -1) {
            perror("read");
            exit(EXIT_FAILURE);
        }
        buffer[bytes_read] = '\0'; // Null-terminate the string

        // Reverse the case of the message
        reverse_case(buffer);

        // Print the modified message
        printf("Child received and modified message: %s\n", buffer);

        close(pipefd[0]); // Close the read end of the pipe
        exit(EXIT_SUCCESS);
    } else {
        // Parent process
        close(pipefd[0]); // Close the read end of the pipe

        // Get the message to send
        printf("Enter a message: ");
        fgets(buffer, BUFFER_SIZE, stdin);
        buffer[strcspn(buffer, "\n")] = '\0'; // Remove trailing newline

        // Write the message to the pipe
        if (write(pipefd[1], buffer, strlen(buffer)) == -1) {
            perror("write");
            exit(EXIT_FAILURE);
        }

        close(pipefd[1]); // Close the write end of the pipe

        // Wait for the child process to finish
        wait(NULL);
    }

    return 0;
}
