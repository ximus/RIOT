/*
 * Copyright (C) 2015 Kaspar Schleiser <kaspar@schleiser.de>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     tests
 * @{
 *
 * @file
 * @brief       timer test application
 *
 * @author      Kaspar Schleiser <kaspar@schleiser.de>
 *
 * @}
 */

#include <stdio.h>

#include "msg.h"
#include "thread.h"
#include "wtimer.h"

#define N 3

int main(void)
{
    puts("wtimer_usleep_until test application.\n");

    kernel_pid_t me = thread_getpid();

    for (int n = 0; n < N; n++) {
        printf("Setting %u timers, removing timer %u/%u\n", N, n, N);
        wtimer_t timers[N];
        msg_t msg[N];
        for (int i = 0; i < N; i++) {
            msg[i].type = i;
            wtimer_set_msg(&timers[i], 100000*(i+1), &msg[i], me);
        }

        wtimer_remove(&timers[n]);

        int num = N-1;
        while(num--) {
            msg_t m;
            msg_receive(&m);
            if (m.type == n) {
                printf("ERROR: msg type=%i unexpected!\n", m.type);
                return -1;
            }
            else {
                printf("timer %u triggered.\n", m.type);
            }
        }
    }

    printf("test successful.\n");

    return 0;
}
