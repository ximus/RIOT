#include "thread.h"
#include "led.h"

/**
 * some calls to `apply_state` could be avoided, but it shouldn't be
 * an expensive operation, let's keep the code clearer.
 */

#if NUM_LEDS > 8
#error "led: module supports 8 leds at most"
#endif

typedef struct client_t
{
    kernel_pid_t pid;
    struct client_t *prev; /* linked list */
    uint8_t state;
}
client_t;

static client_t clients[LED_CLIENTS_MAX];
static client_t *cur_client = NULL;


extern void led_arch_apply_state(uint8_t);
#define apply_state led_arch_apply_state

static client_t *find_client(void)
{
    kernel_pid_t me = thread_getpid();

    /* for every client */
    for (int i = 0; i < LED_CLIENTS_MAX; ++i)
    {
        if (clients[i].pid == me)
        {
            return &clients[i];
        }
    }
    return NULL;
}

static client_t *new_client(void)
{
    /* for every client */
    for (int i = 0; i < LED_CLIENTS_MAX; ++i)
    {
        if (clients[i].pid == KERNEL_PID_UNDEF &&
            clients[i].prev == NULL)
        {
            clients[i].pid = thread_getpid();
            return &clients[i];
        }
    }
    return NULL;
}

/**
 * walk up the list of clients and set `cur_client` (the list head) to
 * the first client that isn't released (still has pid set).
 */
static int cleanup_clients(void)
{
    if (cur_client == NULL)
    {
        return -1;
    }

    client_t *client = cur_client;

    /* walk up the client aquire history */
    while (client != NULL)
    {
        if (client->pid != KERNEL_PID_UNDEF)
        {
            break;
        }
        client = client->prev;
    }

    cur_client = client;

    if (cur_client != NULL)
    {
        apply_state(cur_client->state);
    }

    return 0;
}

void led_init(void)
{
    for (int i = 0; i < LED_CLIENTS_MAX; ++i)
    {
        clients[i].pid   = KERNEL_PID_UNDEF;
        clients[i].prev  = NULL;
        clients[i].state = 0;
    }
}

int led_aquire(void)
{
    client_t *client = find_client();

    if (client == NULL)
    {
        client = new_client();
    }

    if (client == NULL)
    {
        /* no client slots left */
        return -1;
    }

    /* NOTE: this should be an atomic operation */
    client->prev = cur_client;
    cur_client = client;

    return 0;
}

int led_release(void)
{
    client_t *client = find_client();

    if (client == NULL)
    {
        return -1;
    }

    client->pid = NULL;

    /**
     * if the client is the list head, then this should be an
     * opportunity to reduce the list size
     */
    if (cur_client == client)
    {
        cleanup_clients();
    }

    return 0;
}

int led_on(uint8_t leds)
{
    client_t *client = find_client();

    if (client == NULL)
    {
        return -1;
    }

    client->state |= leds;
    apply_state(client->state);

    return 0;
}

int led_off(uint8_t leds)
{
    client_t *client = find_client();

    if (client == NULL)
    {
        return -1;
    }

    client->state &= ~leds;
    apply_state(client->state);

    return 0;
}

int led_toggle(uint8_t leds)
{
    client_t *client = find_client();

    if (client == NULL)
    {
        return -1;
    }

    client->state ^= leds;
    apply_state(client->state);

    return 0;
}