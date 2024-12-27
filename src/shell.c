/**
 * Yakup Ensar Sayın   - g221210001
 * Tunahan Demircioğlu - g221210373
 * Sude Kocaacar       - g221210050
 * Batuhan Özkanlı     - g221210067
 * Berkay Çongar       - g211210063
 */

#include "../include/shell.h"

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
