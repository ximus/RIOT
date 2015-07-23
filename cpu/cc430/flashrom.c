/*
 * Copyright (C) 2014 INRIA
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup cc430
 * @{
 */

/**
 * @file
 * @brief       cc430 flashrom driver
 *
 * @author      Kévin Roussel <Kevin.Roussel@inria.fr>
 */

#include <stddef.h>
#include <stdint.h>
#include "cpu.h"
#include "irq.h"

static inline uint8_t prepare(void);
static inline void finish(uint8_t istate);
static inline void busy_wait(void);


int flashrom_erase(uint8_t *addr)
{
    uint8_t istate = prepare();

    FCTL3 = FWKEY;                            // Clear Lock bit
    busy_wait();
    FCTL1 = FWKEY+ERASE;                      // Set Erase bit
    *addr = 0;                                // Dummy write to erase Flash seg
    busy_wait();
    FCTL1 = FWKEY;                            // Clear WRT bit
    FCTL3 = FWKEY+LOCK;                       // Set LOCK bit

    finish(istate);
    return 1;
}

// could take advantage of cc430 long-word and block write modes. Not
// necessary for current application.
int flashrom_write(uint8_t *dst, const uint8_t *src, size_t size)
{
    unsigned int i;

    FCTL3 = FWKEY;                           /* Lock = 0 */
    busy_wait();

    for (i = size; i > 0; i--) {
        FCTL1 = FWKEY+WRT;                // Enable byte write
        *(dst++) = *(src++);              /* program Flash word */
    }

    busy_wait();
    FCTL1 = FWKEY;                            // Clear WRT bit
    FCTL3 = FWKEY+LOCK;                       // Set LOCK bit
    return 1;
}


static inline uint8_t prepare(void)
{
    // no need to disable watchdog timer, not enabled by platform
    return disableIRQ();
}

static inline void finish(uint8_t istate)
{
    restoreIRQ(istate);
}

static inline void busy_wait(void)
{
    /* Wait for BUSY = 0, not needed unless run from RAM */
    while (FCTL3 & BUSY) {
        __no_operation();
    }
}
