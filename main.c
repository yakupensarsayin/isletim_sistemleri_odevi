#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <fcntl.h>

#define MAX_ARGS 100
#define MAX_LEN 100

int main()
{
    char input[MAX_LEN];  // kullanicidan alinacak tam komut
    char *args[MAX_ARGS]; // arguman dizisi, execvp'de kullanilacak
    int i = 0;            // asagidaki dongude kullanilacak

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

        // cikis yapmak istiyorsa cik
        if (strcmp(input, "quit") == 0)
            break;

        // bosluklara gore input'u tokenize ediyoruz
        char *token = strtok(input, " ");

        char *input_file = NULL;

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

            args[i++] = token;
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

            // gelen komutu calistir
            execvp(args[0], args);
            // basarisiz ise hata mesaji ve cikis
            perror("Hata: ");
            exit(EXIT_FAILURE);
        }
        else if (pid > 0)
        {
            wait(NULL); // ebeveyn proses cocugu beklesin
        }
        else
        {
            // herhangi bir sebepten oturu hata cikarsa:
            perror("Fork basarisiz oldu.");
        }
    }

    exit(EXIT_SUCCESS);
}
