/**
 * Yakup Ensar Sayın   - g221210001
 * Tunahan Demircioğlu - g221210373
 * Sude Kocaacar       - g221210050
 * Batuhan Özkanlı     - g221210067
 * Berkay Çongar       - g211210063
 */

#ifndef SHELL_H
#define SHELL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_ARGS 100
#define MAX_LEN 1000
#define MAX_CMDS 100

extern pid_t background_pids[MAX_CMDS];
extern int bg_count;

void execute_single_command(char *command, int background);
void execute_pipe(char *commands[], int cmd_count, int background);

#endif