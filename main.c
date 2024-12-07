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

// Arka plan süreçlerini takip etmek için liste
pid_t background_pids[MAX_CMDS];
int bg_count = 0;

void sigchld_handler(int);
void execute_pipe(char *commands[], int cmd_count, int background);
void execute_single_command(char *command, int background);

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

        // Komutun arka planda çalıştırılması için kontrol
        int background = 0;
        if (input[newline_index - 1] == '&')
        {
            background = 1;
            input[newline_index - 1] = 0; // '&' işaretini kaldır
        }

        // Noktalı virgül ile ayrılmış
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
            // Pipe kontrolü
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

    exit(EXIT_SUCCESS);
}

// Arka plan süreçlerini izlemek için
void sigchld_handler(int sig)
{
    int saved_errno = errno;
    int status;
    pid_t pid;

    // Sadece arka plan süreçlerini bekle
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0)
    {
        for (int i = 0; i < bg_count; i++)
        {
            if (background_pids[i] == pid)
            {
                printf("\n[%d] retval: %d\n", pid, WEXITSTATUS(status));
                fflush(stdout);
                background_pids[i] = -1; // Süreç tamamlandı
                break;
            }
        }
    }
    errno = saved_errno;
}

// Tekli komut çalıştırma
void execute_single_command(char *command, int background)
{
    char *args[MAX_ARGS];
    char *input_file = NULL;
    char *output_file = NULL;
    int arg_count = 0;

    char *token = strtok(command, " ");
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
            args[arg_count++] = token;
        }
        token = strtok(NULL, " ");
    }
    args[arg_count] = NULL;

    pid_t pid = fork();

    if (pid == 0)
    {
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
        if (background)
        {
            background_pids[bg_count++] = pid;
            printf("[%d] arka planda çalışıyor\n", pid);
        }
        else
        {
            waitpid(pid, NULL, 0);
        }
    }
    else
    {
        perror("Fork başarısız oldu");
    }
}

// Pipe işlemi
void execute_pipe(char *commands[], int cmd_count, int background)
{
    int pipes[2 * (cmd_count - 1)];
    pid_t pids[MAX_CMDS];
    int i;
    char *input_file = NULL;
    char *output_file = NULL;

    // Gerekli pipe'ları oluştur
    for (i = 0; i < cmd_count - 1; i++)
    {
        if (pipe(pipes + 2 * i) < 0)
        {
            perror("Pipe oluşturulamadı");
            return;
        }
    }

    for (i = 0; i < cmd_count; i++)
    {
        pids[i] = fork();
        if (pids[i] == 0)
        {
            // İlk komut için giriş yönlendirmesi
            if (i == 0)
            {
                char *token = strtok(commands[i], " ");
                char *args[MAX_ARGS];
                int arg_count = 0;

                while (token != NULL)
                {
                    if (strcmp(token, "<") == 0)
                    {
                        token = strtok(NULL, " ");
                        if (token)
                            input_file = token;
                    }
                    else
                    {
                        args[arg_count++] = token;
                    }
                    token = strtok(NULL, " ");
                }
                args[arg_count] = NULL;

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

                if (i < cmd_count - 1)
                {
                    dup2(pipes[2 * i + 1], STDOUT_FILENO);
                }

                for (int j = 0; j < 2 * (cmd_count - 1); j++)
                {
                    close(pipes[j]);
                }

                execvp(args[0], args);
                perror("Hata");
                exit(EXIT_FAILURE);
            }
            // Son komut için çıkış yönlendirmesi
            else if (i == cmd_count - 1)
            {
                char *token = strtok(commands[i], " ");
                char *args[MAX_ARGS];
                int arg_count = 0;

                while (token != NULL)
                {
                    if (strcmp(token, ">") == 0)
                    {
                        token = strtok(NULL, " ");
                        if (token)
                            output_file = token;
                    }
                    else
                    {
                        args[arg_count++] = token;
                    }
                    token = strtok(NULL, " ");
                }
                args[arg_count] = NULL;

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

                if (i > 0)
                {
                    dup2(pipes[2 * (i - 1)], STDIN_FILENO);
                }

                for (int j = 0; j < 2 * (cmd_count - 1); j++)
                {
                    close(pipes[j]);
                }

                execvp(args[0], args);
                perror("Hata");
                exit(EXIT_FAILURE);
            }
            // Ortadaki komutlar
            else
            {
                if (i > 0)
                {
                    dup2(pipes[2 * (i - 1)], STDIN_FILENO);
                }
                if (i < cmd_count - 1)
                {
                    dup2(pipes[2 * i + 1], STDOUT_FILENO);
                }

                for (int j = 0; j < 2 * (cmd_count - 1); j++)
                {
                    close(pipes[j]);
                }

                char *args[MAX_ARGS];
                int arg_count = 0;
                char *token = strtok(commands[i], " ");
                while (token != NULL)
                {
                    args[arg_count++] = token;
                    token = strtok(NULL, " ");
                }
                args[arg_count] = NULL;

                execvp(args[0], args);
                perror("Hata");
                exit(EXIT_FAILURE);
            }
        }
        else if (pids[i] < 0)
        {
            perror("Fork başarısız oldu");
            return;
        }
    }

    // Ebeveyn süreç: tüm pipe'ları kapat
    for (i = 0; i < 2 * (cmd_count - 1); i++)
    {
        close(pipes[i]);
    }

    if (!background)
    {
        for (i = 0; i < cmd_count; i++)
        {
            waitpid(pids[i], NULL, 0);
        }
    }
    else
    {
        for (i = 0; i < cmd_count; i++)
        {
            background_pids[bg_count++] = pids[i];
        }
    }
}
