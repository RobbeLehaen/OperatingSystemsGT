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
        close(log_pipe[1]);

        FILE *log_file = fopen("gateway.log", "a");
        if (!log_file) {
            perror("fopen");
            exit(EXIT_FAILURE);
        }

        char buffer[BUFFER_SIZE];
        int seq_num = 0;
        char timestamp[128];

        while (1) {
            ssize_t bytes_read = read(log_pipe[0], buffer, BUFFER_SIZE);
            if (bytes_read <= 0) {
                break;
            }

            buffer[bytes_read] = '\0';

            time_t now = time(NULL);
            struct tm *tm_info = localtime(&now);
            strftime(timestamp, sizeof(timestamp), "%a %b %d %H:%M:%S %Y", tm_info);

            fprintf(log_file, "%d - %s - %s\n", seq_num++, timestamp, buffer);
            fflush(log_file);
        }

        fclose(log_file);
        close(log_pipe[0]);
        exit(EXIT_SUCCESS);
    } else {
        close(log_pipe[0]);
        return 0;
    }
}

int write_to_log_process(char *msg) {
    if (logger_pid == -1) {
        printf("Error: Logger process not created.\n");
        return -1;
    }

    size_t len = strlen(msg);
    if (write(log_pipe[1], msg, len) != (ssize_t)len) {
        perror("write");
        return -1;
    }

    return 0;
}

int end_log_process() {
    if (logger_pid == -1) {
        return -1;
    }

    close(log_pipe[1]);
    waitpid(logger_pid, NULL, 0);
    logger_pid = -1;
    return 0;
}