#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>

int tokenize(char *string) {
    while(*string) {
        if (isspace(*string)) {
            string++;
            continue;
        }

        if (*string == '+' || *string == '-') {
            string++;
            continue;
        }

        if (isdigit(*string)) {
            long n = strtol(string, &string, 10);
            printf("%ld\n", n);
            continue;
        }

        continue;
    }
}

int main (int argc, char **argv) {
    if (argc != 2) {
        // error("Invalid inpusts");
        return 1;
    }

    char *string;
    string = "Hello World !!!";

    printf("%s\n", string);

    int token = tokenize(argv[1]);
}
