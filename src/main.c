#define _POSIX_C_SOURCE 200809L
#include "../include/shell.h"
#include "../include/signal_handler.h"
#include <signal.h>

pid_t background_pids[MAX_CMDS];
int bg_count = 0;

int main()
{
    struct sigaction sa;
    sa.sa_handler = sigchld_handler;
    sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;
    sigaction(SIGCHLD, &sa, NULL);

    char input[MAX_LEN];

    while (1)
    {
        printf("> ");
        fflush(stdout);

        if (!fgets(input, MAX_LEN, stdin))
        {
            perror("Input error");
            continue;
        }

        int newline_index = strcspn(input, "\n");
        input[newline_index] = 0;

        if (strlen(input) == 0)
            continue;

        if (strcmp(input, "quit") == 0)
        {
            int status;
            pid_t pid;
            while ((pid = waitpid(-1, &status, 0)) > 0)
            {
                printf("[%d] retval: %d\n", pid, WEXITSTATUS(status));
            }
            break;
        }

        int background = 0;
        if (input[newline_index - 1] == '&')
        {
            background = 1;
            input[newline_index - 1] = 0;
        }

        char *commands[MAX_CMDS];
        int cmd_count = 0;

        char *cmd = strtok(input, ";");
        while (cmd != NULL)
        {
            commands[cmd_count++] = cmd;
            cmd = strtok(NULL, ";");
        }

        for (int i = 0; i < cmd_count; i++)
        {
            if (strchr(commands[i], '|'))
            {
                char *pipe_commands[MAX_CMDS];
                int pipe_count = 0;

                char *pipe_cmd = strtok(commands[i], "|");
                while (pipe_cmd != NULL)
                {
                    pipe_commands[pipe_count++] = pipe_cmd;
                    pipe_cmd = strtok(NULL, "|");
                }
                execute_pipe(pipe_commands, pipe_count, background);
            }
            else
            {
                execute_single_command(commands[i], background);
            }
        }
    }

    return EXIT_SUCCESS;
}
