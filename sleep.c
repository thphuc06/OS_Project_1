#include "kernel/types.h"
#include "user/user.h"

int validNumber(char *str) {
    if (str == 0 || *str == '\0') {
        return 0;  
    }
    for (int i = 0; str[i] != '\0'; i++) {
        if (str[i] < '0' || str[i] > '9') {
            return 0;  
        }
    }
    return 1;  
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(2, "Usage: sleep <ticks>\n");
        exit(1);
    }
    if (!validNumber(argv[1])) {
        fprintf(2, "Error: Invalid number. Please enter a positive integer.\n");
        exit(1);
    }
    int ticks = atoi(argv[1]);
    printf("(nothing happens for a little while)\n");
    sleep(ticks);
    exit(0);
}
