#include <stdint.h>
#include "annex/lin.h"

int main(void)
{
    /* TODO: initialize board and enable LIN loopback mode */
    annex_lin_init();

    while (1) {
        /* TODO: echo received LIN frames back on the bus */
        annex_lin_process();
    }

    return 0;
}
