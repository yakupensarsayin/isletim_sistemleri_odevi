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
#define MAX_LEN 100

void sigchld_handler(int);

int main()
{
    struct sigaction sa;
    sa.sa_handler = sigchld_handler;
    sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;
    sigaction(SIGCHLD, &sa, NULL);

    char input[MAX_LEN]; // kullanicidan alinacak tam komut

    while (1)
    {
        printf("> ");
        fflush(stdout);

        // Eger bir sebepten oturu input stdin'den alinamazsa:
        if (!fgets(input, MAX_LEN, stdin))
        {
            perror("Girdi hatasi");
            continue;
        }

        // satir sonunun bulundugu yeri buluyoruz
        int newline_index = strcspn(input, "\n");
        // onu null terminator ile degistiriyoruz
        // execvp'nin isleyisini bozmasin diye
        input[newline_index] = 0;

        // eger input bossa devam et
        if (strlen(input) == 0)
            continue;

        if (strcmp(input, "quit") == 0)
        {
            // eger varsa arka plan islemlerini bekle
            int status;
            pid_t pid;
            while ((pid = waitpid(-1, &status, 0)) > 0)
            {
                printf("[%d] retval: %d\n", pid, WEXITSTATUS(status));
            }
            break; // kabuk cikisi
        }

        // arka planda mi calistirilacagini kontrol et
        int background = 0;
        if (input[newline_index - 1] == '&')
        {
            background = 1;
            input[newline_index - 1] = 0; // '&' karakterini kaldir
        }

        // bosluklara gore input'u tokenize ediyoruz
        char *token = strtok(input, " ");

        int i = 0;            // asagidaki dongude kullanilacak
        char *args[MAX_ARGS]; // arguman dizisi, execvp'de kullanilacak
        char *input_file = NULL;
        char *output_file = NULL;

        while (token != NULL)
        {
            if (strcmp(token, "<") == 0)
            {
                token = strtok(NULL, " "); // dosya adini almaya calisiyoruz
                if (token != NULL)
                {
                    input_file = token;
                }
            }
            else if (strcmp(token, ">") == 0)
            {
                token = strtok(NULL, " "); // dosya adini almaya calisiyoruz
                if (token != NULL)
                {
                    output_file = token;
                }
            }
            else
            {
                args[i++] = token;
            }
            token = strtok(NULL, " ");
        }

        // execvp'teki argumanin sonuncusu NULL ile bitmeli
        // elimizle yerlestiriyoruz
        args[i] = NULL;

        pid_t pid = fork();

        if (pid == 0)
        {
            // eger girdi dosyasi mevcutsa
            if (input_file)
            {
                // onu acmaya calis
                int in_fd = open(input_file, O_RDONLY);
                if (in_fd < 0)
                {
                    perror("Giris dosyasi acilamadi");
                    exit(EXIT_FAILURE);
                }
                // onu execvp'ye aktar
                dup2(in_fd, STDIN_FILENO);
                close(in_fd);
            }

            // eger cikis dosyasi mevcutsa
            if (output_file)
            {
                // onu yazma modunda ac
                // yoksa olustur
                int out_fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (out_fd < 0)
                {
                    perror("Cikis dosyasi acilamadi");
                    exit(1);
                }
                // yonlendirmeyi yap
                dup2(out_fd, STDOUT_FILENO);
                close(out_fd);
            }

            // gelen komutu calistir
            execvp(args[0], args);
            // basarisiz ise hata mesaji ve cikis
            perror("Hata");
            exit(EXIT_FAILURE);
        }
        else if (pid > 0)
        {
            if (!background)
            {
                waitpid(pid, NULL, 0); // eger arka plan islemi degilse ebeveyn cocugu beklesin
            }
            else
            {
                printf("[%d] basladi\n", pid); // obur turlu arka plan isleminin basladigini bildir
            }
        }
        else
        {
            // herhangi bir sebepten oturu hata cikarsa:
            perror("Fork basarisiz oldu.");
        }
    }

    exit(EXIT_SUCCESS);
}

void sigchld_handler(int sig)
{
    int saved_errno = errno;
    int status;
    pid_t pid;

    // biten cocuk surecleri ekrana yazdiriyoruz
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0)
    {
        printf("\n[%d] retval: %d\n", pid, WEXITSTATUS(status));
        fflush(stdout);
    }
    errno = saved_errno;
}
