#include "logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/wait.h>

#define BUFFER_SIZE 1024

static int log_pipe[2];
static pid_t logger_pid = -1;

int create_log_process() {
    if (pipe(log_pipe) == -1) {
        perror("pipe");
        return -1;
    }

    logger_pid = fork();
    if (logger_pid == -1) {
        perror("fork");
        return -1;
    }

    if (logger_pid == 0) {
        // Child process (logger)
        close(log_pipe[1]); // Close write end

        FILE *log_file = fopen("gateway.log", "a");
        if (!log_file) {
            perror("fopen");
            exit(EXIT_FAILURE);
        }

        char buffer[BUFFER_SIZE];
        int seq_num = 0;

        while (1) {
            ssize_t bytes_read = read(log_pipe[0], buffer, BUFFER_SIZE);
            if (bytes_read <= 0) {
                break; // Exit if pipe is closed
            }

            buffer[bytes_read] = '\0'; // Null-terminate the string

            // Get current timestamp
            time_t now = time(NULL);
            char *time_str = ctime(&now);
            time_str[strlen(time_str) - 1] = '\0'; // Remove the newline character

            // Log message in the required format: "sequence number - timestamp - message"
            fprintf(log_file, "%d - %s - %s\n", seq_num++, time_str, buffer);
            fflush(log_file); // Ensure the log is written immediately
        }

        fclose(log_file);
        close(log_pipe[0]); // Close read end
        exit(EXIT_SUCCESS);
    } else {
        // Parent process
        close(log_pipe[0]); // Close read end
        return 0;
    }
}

int write_to_log_process(char *msg) {
    if (logger_pid == -1) {
        return -1; // Logger process not created yet
    }

    // Ensure each log message ends with a newline
    size_t len = strlen(msg);
    char *msg_with_newline = malloc(len + 2); // Allocate space for the message and a newline
    if (!msg_with_newline) {
        perror("malloc");
        return -1;
    }

    strcpy(msg_with_newline, msg);
    msg_with_newline[len] = '\n';  // Add newline at the end
    msg_with_newline[len + 1] = '\0'; // Null terminate the string

    size_t msg_len = strlen(msg_with_newline);
    ssize_t written = write(log_pipe[1], msg_with_newline, msg_len);
    free(msg_with_newline);  // Free the allocated memory after writing

    if (written != (ssize_t)msg_len) {
        perror("write");
        return -1;
    }

    // Ensure the log message is flushed to the log file immediately
    fflush(stdout);  // Flush the output buffer
    return 0;
}

int end_log_process() {
    if (logger_pid == -1) {
        return -1; // Logger process not created yet
    }

    close(log_pipe[1]); // Close write end to signal EOF to the logger
    waitpid(logger_pid, NULL, 0); // Wait for logger to terminate
    logger_pid = -1;
    return 0;
}
