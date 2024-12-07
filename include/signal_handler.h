#ifndef SIGNAL_HANDLER_H
#define SIGNAL_HANDLER_H

#include <sys/wait.h>
#include <stdio.h>
#include <errno.h>

void sigchld_handler(int sig);

#endif