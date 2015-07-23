/*
 * Copyright (C) 2015 Martine Lenders <mlenders@inf.fu-berlin.de>
 *
 * This file is subject to the terms and conditions of the GNU Lesser General
 * Public License v2.1. See the file LICENSE in the top level directory for
 * more details.
 */

/**
 * @ingroup     sys_shell_commands
 * @{
 *
 * @file
 *
 * @author      Martine Lenders <mlenders@inf.fu-berlin.de>
 */

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

#ifdef MODULE_NG_ICMPV6

#include "byteorder.h"
#include "net/ng_icmpv6.h"
#include "net/ng_ipv6/addr.h"
#include "net/ng_ipv6/nc.h"
#include "net/ng_ipv6/hdr.h"
#include "net/ng_netbase.h"
#include "thread.h"
#include "utlist.h"
#include "vtimer.h"

static uint16_t id = 0x53;
static uint16_t min_seq_expected = 0;
static uint16_t max_seq_expected = 0;
static char ipv6_str[NG_IPV6_ADDR_MAX_STR_LEN];

static void usage(char **argv)
{
    printf("%s [<count>] <ipv6 addr> [<payload_len>] [<delay in ms>]\n", argv[0]);
    puts("defaults:");
    puts("    count = 3");
    puts("    payload_len = 4");
    puts("    delay = 1000");
}

void _set_payload(ng_icmpv6_echo_t *hdr, size_t payload_len)
{
    size_t i = 0;
    uint8_t *payload = (uint8_t *)(hdr + 1);

    while (i < payload_len) {
        if (id > 0xff) {
            payload[i] = (uint8_t)(id >> 8);
            payload[i + 1] = (uint8_t)(id & 0xff);
            i += 2;
        }
        else {
            payload[i++] = (uint8_t)(id & 0xff);
        }
    }

    if (i > payload_len) {  /* when id > 0xff and payload_len is odd */
        payload[payload_len - 1] = id >> 8;
    }
}

static inline bool _expected_seq(uint16_t seq)
{
    /* take integer overflows in account */
    if (min_seq_expected > max_seq_expected) {
        return (seq >= min_seq_expected) || (seq <= max_seq_expected);
    }
    else {
        return (seq >= min_seq_expected) && (seq <= max_seq_expected);
    }
}

int _handle_reply(ng_pktsnip_t *pkt, uint64_t time)
{
    ng_pktsnip_t *ipv6, *icmpv6;
    ng_ipv6_hdr_t *ipv6_hdr;
    ng_icmpv6_echo_t *icmpv6_hdr;
    uint16_t seq;

    LL_SEARCH_SCALAR(pkt, ipv6, type, NG_NETTYPE_IPV6);
    LL_SEARCH_SCALAR(pkt, icmpv6, type, NG_NETTYPE_ICMPV6);

    if ((ipv6 == NULL) || (icmpv6 == NULL)) {
        puts("error: IPv6 header or ICMPv6 header not found in reply");
        return 0;
    }

    ipv6_hdr = ipv6->data;
    icmpv6_hdr = icmpv6->data;
    seq = byteorder_ntohs(icmpv6_hdr->seq);

    if ((byteorder_ntohs(icmpv6_hdr->id) == id) && _expected_seq(seq)) {
        if (seq <= min_seq_expected) {
            min_seq_expected++;
        }

        timex_t rt = timex_from_uint64(time);
        printf("%u bytes from %s: id=%" PRIu16 " seq=%" PRIu16 " hop limit=%" PRIu8
               " time = %" PRIu32 ".%03" PRIu32 " ms\n", (unsigned) icmpv6->size,
               ng_ipv6_addr_to_str(ipv6_str, &(ipv6_hdr->src), sizeof(ipv6_str)),
               byteorder_ntohs(icmpv6_hdr->id), seq, ipv6_hdr->hl,
               (rt.seconds * SEC_IN_MS) + (rt.microseconds / MS_IN_USEC),
               rt.microseconds % MS_IN_USEC);
        ng_ipv6_nc_still_reachable(&ipv6_hdr->src);
    }
    else {
        puts("error: unexpected parameters");
        return 0;
    }

    return 1;
}

int _icmpv6_ping(int argc, char **argv)
{
    int count = 3, success = 0, remaining;
    size_t payload_len = 4;
    timex_t delay = { 1, 0 };
    char *addr_str;
    ng_ipv6_addr_t addr;
    ng_netreg_entry_t *ipv6_entry, my_entry = { NULL, NG_ICMPV6_ECHO_REP,
                                                thread_getpid()
                                              };
    timex_t min_rtt = { UINT32_MAX, UINT32_MAX }, max_rtt = { 0, 0 };
    timex_t sum_rtt = { 0, 0 };
    timex_t start, stop;

    switch (argc) {
        case 0:
        case 1:
            usage(argv);
            return 1;

        case 2:
            addr_str = argv[1];
            break;

        case 3:
            count = atoi(argv[1]);
            if (count > 0) {
                addr_str = argv[2];
            }
            else {
                count = 3;
                addr_str = argv[1];
                payload_len = atoi(argv[2]);
            }

            break;

        case 4:
            count = atoi(argv[1]);
            if (count > 0) {
                addr_str = argv[2];
                payload_len = atoi(argv[3]);
            }
            else {
                count = 3;
                addr_str = argv[1];
                payload_len = atoi(argv[2]);
                delay.seconds = 0;
                delay.microseconds = atoi(argv[3]) * 1000;
                timex_normalize(&delay);
            }
            break;

        case 5:
        default:
            count = atoi(argv[1]);
            addr_str = argv[2];
            payload_len = atoi(argv[3]);
            delay.seconds = 0;
            delay.microseconds = atoi(argv[4]) * 1000;
            timex_normalize(&delay);
            break;
    }

    if (ng_ipv6_addr_from_str(&addr, addr_str) == NULL) {
        usage(argv);
        return 1;
    }

    if (ng_netreg_register(NG_NETTYPE_ICMPV6, &my_entry) < 0) {
        puts("error: network registry is full");
        return 1;
    }

    ipv6_entry = ng_netreg_lookup(NG_NETTYPE_IPV6, NG_NETREG_DEMUX_CTX_ALL);

    if (ipv6_entry == NULL) {
        puts("error: ipv6 thread missing");
        return 1;
    }

    remaining = count;

    vtimer_now(&start);

    while ((remaining--) > 0) {
        msg_t msg;
        ng_pktsnip_t *pkt;
        timex_t start, stop, timeout = { 5, 0 };

        pkt = ng_icmpv6_echo_req_build(id, ++max_seq_expected, NULL,
                                       payload_len);

        if (pkt == NULL) {
            puts("error: packet buffer full");
            return 1;
        }

        _set_payload(pkt->data, payload_len);

        pkt = ng_netreg_hdr_build(NG_NETTYPE_IPV6, pkt, NULL, 0, addr.u8,
                                  sizeof(ng_ipv6_addr_t));

        if (pkt == NULL) {
            puts("error: packet buffer full");
            return 1;
        }

        vtimer_now(&start);
        ng_netapi_send(ipv6_entry->pid, pkt);

        if (vtimer_msg_receive_timeout(&msg, timeout) >= 0) {
            switch (msg.type) {
                case NG_NETAPI_MSG_TYPE_RCV:
                    vtimer_now(&stop);
                    stop = timex_sub(stop, start);

                    ng_pktsnip_t *pkt = (ng_pktsnip_t *)msg.content.ptr;
                    success += _handle_reply(pkt, timex_uint64(stop));
                    ng_pktbuf_release(pkt);

                    if (timex_cmp(stop, max_rtt) > 0) {
                        max_rtt = stop;
                    }

                    if (timex_cmp(stop, min_rtt) < 1) {
                        min_rtt = stop;
                    }

                    sum_rtt = timex_add(sum_rtt, stop);

                    break;

                default:
                    /* requeue wrong packets */
                    msg_send(&msg, thread_getpid());
                    break;
            }
        }
        else {
            puts("ping timeout");
        }

        if (remaining > 0) {
            vtimer_sleep(delay);
        }
    }

    vtimer_now(&stop);

    max_seq_expected = 0;
    id++;
    stop = timex_sub(stop, start);

    ng_netreg_unregister(NG_NETTYPE_ICMPV6, &my_entry);

    printf("--- %s ping statistics ---\n", addr_str);

    if (success > 0) {
        timex_normalize(&sum_rtt);
        printf("%d packets transmitted, %d received, %d%% packet loss, time %"
               PRIu32 ".06%" PRIu32 " s\n", count, success,
               (100 - ((success * 100) / count)), stop.seconds, stop.microseconds);
        timex_t avg_rtt = timex_from_uint64(timex_uint64(sum_rtt) / count);  /* get average */
        printf("rtt min/avg/max = "
               "%" PRIu32 ".%03" PRIu32 "/"
               "%" PRIu32 ".%03" PRIu32 "/"
               "%" PRIu32 ".%03" PRIu32 " ms\n",
               (min_rtt.seconds * SEC_IN_MS) + (min_rtt.seconds / MS_IN_USEC),
               min_rtt.microseconds % MS_IN_USEC,
               (avg_rtt.seconds * SEC_IN_MS) + (avg_rtt.seconds / MS_IN_USEC),
               avg_rtt.microseconds % MS_IN_USEC,
               (max_rtt.seconds * SEC_IN_MS) + (max_rtt.seconds / MS_IN_USEC),
               max_rtt.microseconds % MS_IN_USEC);
    }
    else {
        printf("%d packets transmitted, 0 received, 100%% packet loss\n", count);
        return 1;
    }

    return 0;
}

#endif

/**
 * @}
 */
