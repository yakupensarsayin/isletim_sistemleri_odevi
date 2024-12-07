#include "../include/signal_handler.h"
#include "../include/shell.h"

void sigchld_handler(int sig)
{
    int saved_errno = errno;
    int status;
    pid_t pid;

    while ((pid = waitpid(-1, &status, WNOHANG)) > 0)
    {
        for (int i = 0; i < bg_count; i++)
        {
            if (background_pids[i] == pid)
            {
                printf("\n[%d] retval: %d\n", pid, WEXITSTATUS(status));
                fflush(stdout);
                background_pids[i] = -1;
                break;
            }
        }
    }
    errno = saved_errno;
}
