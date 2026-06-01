#include <stdint.h>
#include "annex/lin.h"

int main(void)
{
    /* TODO: initialize LIN responder logic and listen for master requests */
    annex_lin_init();

    while (1) {
        /* TODO: respond to LIN request frames as a responder node */
        annex_lin_process();
    }

    return 0;
}
