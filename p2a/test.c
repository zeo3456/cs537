#include <ctype.h>
#include <stdio.h>
#include <string.h>

int main (int argc, char *argv[]) {
    char line[30] = " asdf   fdsaf fqreqr  ";
    
    int count = 0;
    char *words[512];
    char *input = strdup(line);
    while (1) {
        while (isspace(*input))
            input++;
        if (*input == '\0')
            return count;
        words[count++] = input;
        printf("%s\n", input);
        while (!isspace(*input) && *input != '\0')
            input++;
    }
    
    return 0;
}