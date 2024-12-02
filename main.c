#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

#define MAX_ARGS 100
#define MAX_LEN 100

int main() {
    char input[MAX_LEN]; // kullanicidan alinacak tam komut
    char *args[MAX_ARGS]; // arguman dizisi, execvp'de kullanilacak
    int i; // asagidaki dongude kullanilacak

    while (1) {
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
        if (strlen(input) == 0) continue; 

        // cikis yapmak istiyorsa cik
        if (strcmp(input, "quit") == 0) break;
        
        // bosluklara gore input'u tokenize ediyoruz
        char *token = strtok(input, " "); 

        for (i = 0; i < MAX_ARGS && token != NULL; i++)
        {
            // her bir token'i tek tek args'a yerlestiriyoruz
            args[i] = token;
            // sonra kendisini tekrar bosluklara boluyoruz
            token = strtok(NULL, " ");
        }

        // execvp'teki argumanin sonuncusu NULL ile bitmeli
        // elimizle yerlestiriyoruz
        args[i] = NULL;

        pid_t pid = fork();

        if (pid == 0)
        {
            // gelen komutu calistir
            execvp(args[0], args);
            // basarisiz ise hata mesaji ve cikis
            perror("Hata: ");
            exit(EXIT_FAILURE);
        } else if (pid > 0) {
            wait(NULL); // ebeveyn proses cocugu beklesin
        } else {
            // herhangi bir sebepten oturu hata cikarsa:
            perror("Fork basarisiz oldu.");
        }
   
    }

    exit(EXIT_SUCCESS);
}
