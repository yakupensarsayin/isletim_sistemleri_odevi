#define _POSIX_C_SOURCE 200809L
#include <features.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>

#define MAX_ARGS 100
#define MAX_LEN 1000
#define MAX_CMDS 100

void sigchld_handler(int);

int main()
{
    struct sigaction sa;
    sa.sa_handler = sigchld_handler;
    sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;
    sigaction(SIGCHLD, &sa, NULL);

    char input[MAX_LEN]; // kullanıcıdan alınacak tam komut

    while (1)
    {
        printf("> ");
        fflush(stdout);

        if (!fgets(input, MAX_LEN, stdin))
        {
            perror("Girdi hatası");
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

        // ";" ile komutları ayır
        char *commands[MAX_CMDS];
        int cmd_count = 0;
        char *cmd = strtok(input, ";");
        while (cmd != NULL)
        {
            commands[cmd_count++] = cmd;
            cmd = strtok(NULL, ";");
        }

        // Her komut için işlem yap
        for (int c = 0; c < cmd_count; c++)
        {
            char *current_cmd = commands[c];

            // Arka planda mı çalıştırılacak kontrol et
            int background = 0;
            int len = strlen(current_cmd);
            if (len > 0 && current_cmd[len - 1] == '&')
            {
                background = 1;
                current_cmd[len - 1] = '\0'; // '&' karakterini kaldır
            }

            // Komutu tokenize et
            char *args[MAX_ARGS];
            char *input_file = NULL;
            char *output_file = NULL;
            int i = 0;

            char *token = strtok(current_cmd, " ");
            while (token != NULL)
            {
                if (strcmp(token, "<") == 0)
                {
                    token = strtok(NULL, " ");
                    if (token)
                        input_file = token;
                }
                else if (strcmp(token, ">") == 0)
                {
                    token = strtok(NULL, " ");
                    if (token)
                        output_file = token;
                }
                else
                {
                    args[i++] = token;
                }
                token = strtok(NULL, " ");
            }

            args[i] = NULL;

            pid_t pid = fork();
            if (pid == 0)
            {
                // Girdi yönlendirmesi
                if (input_file)
                {
                    int in_fd = open(input_file, O_RDONLY);
                    if (in_fd < 0)
                    {
                        perror("Giriş dosyası açılamadı");
                        exit(EXIT_FAILURE);
                    }
                    dup2(in_fd, STDIN_FILENO);
                    close(in_fd);
                }

                // Çıktı yönlendirmesi
                if (output_file)
                {
                    int out_fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                    if (out_fd < 0)
                    {
                        perror("Çıkış dosyası açılamadı");
                        exit(EXIT_FAILURE);
                    }
                    dup2(out_fd, STDOUT_FILENO);
                    close(out_fd);
                }

                execvp(args[0], args);
                perror("Hata");
                exit(EXIT_FAILURE);
            }
            else if (pid > 0)
            {
                if (!background)
                {
                    waitpid(pid, NULL, 0); // Arka planda değilse bekle
                }
                else
                {
                    printf("[%d] arka planda başlatıldı\n", pid);
                }
            }
            else
            {
                perror("Fork başarısız oldu.");
            }
        }
    }

    exit(EXIT_SUCCESS);
}

void sigchld_handler(int sig)
{
    int saved_errno = errno;
    int status;
    pid_t pid;

    while ((pid = waitpid(-1, &status, WNOHANG)) > 0)
    {
        printf("\n[%d] tamamlandı, retval: %d\n", pid, WEXITSTATUS(status));
        fflush(stdout);
    }
    errno = saved_errno;
}
