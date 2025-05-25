#include <stdio.h>    // For standard input/output functions (printf, fgets)
#include <stdlib.h>   // For standard library functions (malloc, free, exit)
#include <string.h>   // For string manipulation functions (strcmp, strtok)
#include <unistd.h>   // For POSIX operating system API (fork, execvp)
#include <sys/wait.h> // For waitpid function

// Define the maximum number of arguments a command can have
#define MAX_ARGS 10
// Define the maximum length of a command line
#define MAX_LINE_LENGTH 256

/**
 * @brief Reads a line of input from stdin.
 *
 * @param line_buffer Character array to store the read line.
 * @param buffer_size Maximum size of the buffer.
 * @return 0 on success, -1 on EOF or error.
 */
int read_command_line(char *line_buffer, size_t buffer_size) {
    printf("simple_shell> "); // Display the shell prompt
    fflush(stdout);           // Ensure the prompt is displayed immediately

    // Read a line from standard input
    if (fgets(line_buffer, buffer_size, stdin) == NULL) {
        // If fgets returns NULL, it's either EOF (Ctrl+D) or an error
        if (feof(stdin)) {
            printf("\nExiting shell...\n");
            return -1; // Indicate EOF
        } else {
            perror("fgets error");
            return -1; // Indicate other error
        }
    }

    // Remove the trailing newline character if present
    line_buffer[strcspn(line_buffer, "\n")] = 0;

    return 0; // Success
}

/**
 * @brief Parses a command line string into an array of arguments.
 *
 * This function tokenizes the input string by spaces and stores
 * each token (argument) into the args array. The last element
 * of args is set to NULL, which is required by execvp.
 *
 * @param line_buffer The input command line string.
 * @param args An array of character pointers to store the arguments.
 * @return The number of arguments parsed.
 */
int parse_command_line(char *line_buffer, char *args[]) {
    int arg_count = 0;
    char *token;

    // Get the first token (command name)
    token = strtok(line_buffer, " ");

    // Loop through the string to get all arguments
    while (token != NULL && arg_count < MAX_ARGS - 1) {
        args[arg_count++] = token;
        // Get the next token
        token = strtok(NULL, " ");
    }
    args[arg_count] = NULL; // execvp requires the last argument to be NULL

    return arg_count;
}

/**
 * @brief Displays the help message for the simple shell.
 */
void display_help() {
    printf("--- Simple Shell Help ---\n");
    printf("Available built-in commands:\n");
    printf("  help   : Display this help message.\n");
    printf("  exit   : Terminate the shell.\n");
    printf("\n");
    printf("Other commands are executed via the system's PATH.\n");
    printf("Examples:\n");
    printf("  ls -l\n");
    printf("  echo Hello World\n");
    printf("-------------------------\n");
}

/**
 * @brief Executes a command with its arguments.
 *
 * This function forks a new process. The child process attempts to execute
 * the command using execvp. The parent process waits for the child to complete.
 *
 * @param args An array of character pointers representing the command and its arguments.
 */
void execute_command(char *args[]) {
    pid_t pid; // Process ID variable
    int status; // Status of the child process

    // Check if the command is empty
    if (args[0] == NULL) {
        return; // No command entered, do nothing
    }

    // --- Handle built-in commands ---
    if (strcmp(args[0], "exit") == 0) {
        printf("Exiting simple_shell.\n");
        exit(0); // Terminate the shell
    } else if (strcmp(args[0], "help") == 0) {
        display_help(); // Call the help function
        return; // Built-in command handled, return to shell loop
    }

    // --- Execute external commands ---
    // Fork a new process
    pid = fork();

    if (pid == -1) {
        // Error occurred during forking
        perror("fork error");
        return;
    } else if (pid == 0) {
        // This code runs in the child process

        // Execute the command
        // execvp searches for the executable in the PATH environment variable
        // args[0] is the command, args is the array of arguments
        if (execvp(args[0], args) == -1) {
            // execvp returns -1 only if an error occurs (e.g., command not found)
            perror("execvp error");
            exit(EXIT_FAILURE); // Child process exits with failure status
        }
    } else {
        // This code runs in the parent process

        // Wait for the child process to complete
        // pid: process ID to wait for (our child's PID)
        // &status: pointer to an integer where the child's exit status will be stored
        // 0: options (no special options)
        if (waitpid(pid, &status, 0) == -1) {
            perror("waitpid error");
        }
    }
}

/**
 * @brief Main function for the simple shell.
 *
 * This function contains the main loop of the shell, where it continuously
 * reads commands, parses them, and executes them.
 *
 * @return 0 on successful exit.
 */
int main() {
    char line_buffer[MAX_LINE_LENGTH]; // Buffer to store the command line input
    char *args[MAX_ARGS];              // Array to store command arguments

    // Main shell loop
    while (1) {
        // Read the command line
        if (read_command_line(line_buffer, sizeof(line_buffer)) == -1) {
            break; // Exit loop if EOF or error occurs
        }

        // Parse the command line into arguments
        parse_command_line(line_buffer, args);

        // Execute the command
        execute_command(args);
    }

    return 0; // Shell exits normally
}
