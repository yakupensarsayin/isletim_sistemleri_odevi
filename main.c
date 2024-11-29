#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main() {
    char *args[] = {"ls", "-la", NULL};

    pid_t pid = fork();

    if (pid == 0)
    {
        execvp(args[0], args);
    }
    else if (pid > 0)
    {
        wait(NULL);
        printf("Ana proses devraldi.");
    }

    exit(EXIT_SUCCESS);
}
