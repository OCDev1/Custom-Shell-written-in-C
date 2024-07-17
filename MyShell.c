#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX_COMMAND_LENGTH 100
#define MAX_COMMANDS 100

char *history[MAX_COMMANDS];
int history_count = 0;

void add_to_history(char *command) {
    if (history_count < MAX_COMMANDS) {
        history[history_count++] = strdup(command);
    } else {
        printf("Error: History is full\n");
    }
}

void print_history() {
    for (int i = 0; i < history_count; i++) {
        printf("%s\n", history[i]);
    }
}

void cd(char *path) {
    int change = chdir(path);
    if (change != 0) {
        perror("chdir failed");
    }
}

void pwd() {
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("%s\n", cwd);
    } else {
        perror("getcwd");
    }
}

int main(int argc, char *argv[]) {
    char command[MAX_COMMAND_LENGTH];
    char *token, *args[MAX_COMMAND_LENGTH / 2 + 1];
    int should_run = 1;
    char path[MAX_COMMAND_LENGTH] = "/bin:/usr/bin";

    // Append command-line arguments to the PATH
    for (int i = 1; i < argc; i++) {
        strncat(path, ":", MAX_COMMAND_LENGTH - strlen(path) - 1);
        strncat(path, argv[i], MAX_COMMAND_LENGTH - strlen(path) - 1);
    }
    setenv("PATH", path, 1);

    while (should_run) {
        printf("$ ");
        fflush(stdout);

        if (fgets(command, sizeof(command), stdin) == NULL) {
            break;
        }

        command[strcspn(command, "\n")] = '\0';  // Remove newline character
        add_to_history(command);

        int arg_count = 0;
        token = strtok(command, " ");
        while (token != NULL) {
            args[arg_count++] = token;
            token = strtok(NULL, " ");
        }
        args[arg_count] = NULL;

        if (strcmp(args[0], "history") == 0) {
            print_history();
        } else if (strcmp(args[0], "cd") == 0) {
            if (arg_count == 2) {
                cd(args[1]);
            } else {
                cd("");
            }
        } else if (strcmp(args[0], "pwd") == 0) {
            pwd();
        } else if (strcmp(args[0], "exit") == 0) {
            should_run = 0;
        } else {
            pid_t pid = fork();
            if (pid == -1) {
                perror("fork");
            } else if (pid == 0) {
                // Child process
                if (execvp(args[0], args) == -1) {
                    perror(args[0]);
                    exit(EXIT_FAILURE);
                }
            } else {
                // Parent process
                int status;
                waitpid(pid, &status, 0);
            }
        }
    }

    return 0;
}
