#include <stdint.h>
#include "annex/lin.h"

int main(void)
{
    /* TODO: initialize platform HAL and lin commander application */
    annex_lin_init();

    while (1) {
        /* TODO: generate and send LIN command frames */
        annex_lin_process();
    }

    return 0;
}
