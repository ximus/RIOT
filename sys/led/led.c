#include "thread.h"
#include "led.h"
#include "wtimer.h"

#define MS_IN_USEC  (1000)

#define ENABLE_DEBUG    (1)
#include "debug.h"

/**
 * some calls to `apply_state` could be avoided, but it shouldn't be
 * an expensive operation. Let's keep the code clearer.
 */

#if LEDS_NUMOF > 8
#error "led: module supports 8 leds at most"
#endif

typedef struct client_t
{
    kernel_pid_t pid;
    struct client_t *prev; /* linked list */
    uint8_t  state;
    uint8_t  blink_state;
    uint16_t blink_interval; /* ms */
}
client_t;

static client_t clients[LED_CLIENTS_MAX];
static client_t *cur_client = NULL;

static wtimer_t blink_timer;

/* define this for that special board in you heart */
extern void led_arch_apply_state(uint8_t);
#define apply_state led_arch_apply_state

/* helper forward declerations */
static inline void blink_off(client_t *client, uint8_t leds);
static inline void set_blink_timer(uint16_t interval_ms);
static inline void remove_blink_timer(void);
static void led_blink_timer_callback(void *arg);

/* client helpers, currently clients are one to one mapping with calling threads */
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
static client_t *prev_client(void)
{
    client_t *client = cur_client;

    /* walk up the client aquire history */
    while (client != NULL)
    {
        client_t *prev = client->prev;
        client->prev = NULL; /* important operation to free up client list */
        client = prev;
        if (client->pid != KERNEL_PID_UNDEF)
        {
            break;
        }
    }

    return client;
}

static void set_cur_client(client_t *client)
{
    if (cur_client)
    {
        remove_blink_timer();
        /* unset any blink'ed leds*/
        cur_client->state &= ~cur_client->blink_state;
        /* turn all leds off */
        apply_state(0);

        DEBUG("led: switch from client pid %p\n", cur_client);
    }

    cur_client = client;

    if (cur_client)
    {
        apply_state(cur_client->state);
        /* client might have existing blink leds */
        if (cur_client->blink_state)
        {
            set_blink_timer(client->blink_interval);
        }

        DEBUG("led: switch to client pid %p\n", cur_client);
    }
    else {
        DEBUG("led: switch to client NULL\n");
    }
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

    /* NOTE: this should be an atomic operation ? */
    client->prev = cur_client;
    set_cur_client(client);

    return 0;
}

int led_release(void)
{
    client_t *client = find_client();

    if (client == NULL)
    {
        DEBUG("led_release(): unknown caller\n");
        return -1;
    }

    /* marks client as free, though it may not be reused until client->prev is pop'ed */
    client->pid = KERNEL_PID_UNDEF;

    /* if the client is the list head, then we may move down the client list */
    if (cur_client == client)
    {
        DEBUG("led: current owner released, looking for next client ...\n");
        set_cur_client(prev_client());
    }

    return 0;
}

int led_on(uint8_t leds)
{
    client_t *client = find_client();

    if (client == NULL)
    {
        DEBUG("led_on(): unknown caller");
        return -1;
    }

    blink_off(client, leds);

    client->state |= leds;
    apply_state(client->state);

    return 0;
}

int led_off(uint8_t leds)
{
    client_t *client = find_client();

    if (client == NULL)
    {
        DEBUG("led_off(): unknown caller\n");
        return -1;
    }

    blink_off(client, leds);

    client->state &= ~leds;
    apply_state(client->state);

    return 0;
}

int led_toggle(uint8_t leds)
{
    client_t *client = find_client();

    if (client == NULL)
    {
        DEBUG("led_toggle(): unknown caller\n");
        return -1;
    }

    blink_off(client, leds);

    client->state ^= leds;
    apply_state(client->state);

    return 0;
}

int led_blink(uint8_t leds, uint16_t interval_ms)
{
    client_t *client = find_client();

    if (client == NULL)
    {
        DEBUG("led_blink(): unknown caller\n");
        return -1;
    }

    /* clear timer if running */
    remove_blink_timer();

    /* shut leds off */
    client->state &= ~leds;
    apply_state(client->state);

    /* enable leds for blinking */
    client->blink_state |= leds;
    client->blink_interval = interval_ms;

    set_blink_timer(client->blink_interval);

    return 0;
}

static inline void blink_off(client_t *client, uint8_t leds)
{
    client->blink_state &= ~leds;
}

static inline void set_blink_timer(uint16_t interval_ms)
{
    blink_timer.callback = &led_blink_timer_callback;
    blink_timer.arg = (void *) cur_client;

    wtimer_set(&blink_timer, (uint32_t) interval_ms * MS_IN_USEC);
}

static inline void remove_blink_timer(void)
{
    wtimer_remove(&blink_timer);
}


static void led_blink_timer_callback(void *arg)
{
    client_t *client = (client_t *) arg;

    if (client == cur_client && client->blink_state)
    {
        /* toggle blinking leds*/
        client->state ^= client->blink_state;
        apply_state(client->state);
        set_blink_timer(client->blink_interval);
    }
}