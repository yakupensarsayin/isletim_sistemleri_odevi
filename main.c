#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main() {
    char *args[] = {"ls", "-la", NULL};
    execvp(args[0], args);
    exit(EXIT_SUCCESS);
}
