/**
 * Yakup Ensar Sayın   - g221210001
 * Tunahan Demircioğlu - g221210373
 * Sude Kocaacar       - g221210050
 * Batuhan Özkanlı     - g221210067
 * Berkay Çongar       - g211210063
 */

#ifndef SIGNAL_HANDLER_H
#define SIGNAL_HANDLER_H

#include <sys/wait.h>
#include <stdio.h>
#include <errno.h>

void sigchld_handler(int sig);

#endif