/* signal handler */
#include <clib/signal.h>


sig_handler *signal_install(int signo, sig_handler *func)
{
    struct sigaction act, old_act;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    act.sa_handler = func;
    if (sigaction(signo, &act, &old_act) < 0)
        return SIG_ERR;
    return old_act.sa_handler;
}
