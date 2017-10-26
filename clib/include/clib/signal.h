#ifndef YAMU_SIGNAL_H
#define YAMU_SIGNAL_H

#include <signal.h>
typedef void sig_handler(int);

/*
 * @brief a simple siaction wrapper
 * @param in signo: signal number defined in std signal.h
 * @para in func: signal handler
 * @return SIG_ERR if anything wrong happend, else return the old signal handler
 */
sig_handler *signal_install(int signo, sig_handler *func);

#endif
