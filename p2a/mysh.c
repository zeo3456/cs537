#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define INPUT_SIZE 514

char *words[INPUT_SIZE];
char *tmp_words[INPUT_SIZE];
char *command[INPUT_SIZE];
int is_batch = 0;
int is_redirection = 0;
char *preToken, *postToken;

char prompt_message[7] = "mysh # ";
char error_message[30] = "An error has occurred\n";

void prompt() {
    if (!is_batch)
        write(STDOUT_FILENO, prompt_message, strlen(prompt_message));
}

void error() {
    write(STDERR_FILENO, error_message, strlen(error_message));
}

int split(char *line, char *words[]) {
    int count = 0;
    char *input = strdup(line);
    while (1) {
        while (isspace(*input))
            input++;
        if (*input == '\0')
            return count;
        words[count++] = input;
        while (!isspace(*input) && *input != '\0')
            input++;
        if (*input == '\0') return count;
        *input++ = '\0';
    }
}

int main (int argc, char *argv[]) {
    // Variables
    FILE *inFile;
    int outFile, outFile_1;
    char input[INPUT_SIZE];
    int count = 0, command_count = 0, i, status;
    pid_t child, child_wait;
    
    // Interactive or batch mode
    if (argc == 1)
        inFile = stdin;
    else if (argc == 2) {
        inFile = fopen(argv[1], "r");
        if (inFile == NULL) {
            error();
            exit(1);
        }
        is_batch = 1;
    }
    else {
        error();
        exit(1);
    }
    
    prompt();
    
    // Handle input
    while (fgets(input, INPUT_SIZE, inFile)) {
        
        // Input too long
        if (strlen(input) >= 513) {
            if (is_batch && input[strlen(input) - 1] == '\n')
                write(STDOUT_FILENO, input, strlen(input));
            error();
            prompt();
            continue;
        }
        
        // Empty command line
        if (split(input, tmp_words) == 0) {
            prompt();
            continue;
        }
        
        is_redirection = 0;
        preToken = NULL;
        postToken = NULL;
        
        if (is_batch)
            write(STDOUT_FILENO, input, strlen(input));
        
        // Search for redirection
        preToken = strtok(input, ">");
        if (strlen(preToken) != strlen(input)) {
            is_redirection = 1;
            postToken = strtok(NULL, ">");
            // Too many redirections
            if (strtok(NULL, ">")) {
                error();
                prompt();
                continue;
            }
        }
        
        // Handle redirection
        if (!is_redirection)
            count = split(input, words);
        else {
            count = split(preToken, words);
            i = split(postToken, &words[count]);
            count += i;
        }
        
        if (is_redirection) {
            outFile_1 = dup(1);
            outFile = open(words[count - 1], O_WRONLY|O_CREAT|O_TRUNC, S_IRWXU);
            if (outFile < 0) {
                error();
                prompt();
                continue;
            }
            else if (dup2(outFile, 1) < 0) {
                error();
                prompt();
                continue;
            }
        }
        
        // Exit
        if (!strcmp("exit", words[0])) {
            if (count != 1) {
                error();
                prompt();
                dup2(outFile, 1);
                continue;
            }
            else
                exit(0);
        }
        
        if (is_redirection) {
            for (i = 0; i < count - 1; i++)
                command[i] = strdup(words[i]);
            command_count = i;
            command[command_count] = NULL;
        }
        else {
            for (i = 0; i < count; i++)
                command[i] = strdup(words[i]);
            command_count = i;
            command[command_count] = NULL;
        }
        
        child = fork();
        if (child == 0) {
            execvp(command[0], command);
            error();
        }
        else if (child == (pid_t)-1)
            error();
        else
            child_wait = wait(&status);
        
        dup2(outFile, 1);
        prompt();
    }
    
    return 0;
}