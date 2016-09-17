/**
 * Initialize DMA. "rx_callback" will be called for each
 * UART character received when not receiving in block mode.
 */
void uart_init(void);
void uart_on_char_receive(void (*rx_callback)(uint8_t));
/**
 * nomenclature: block basically means dma, should
 * I call it uart_dma_ ...?
 */
/**
 * Reveive from UART directly to a the address "dest" for up to
 * "size" bytes of data, at which point "callback" will be invoked.
 * Reception can be interuppted with "uart_block_receive_end()" and
 * the count of bytes received so far is returned
 * by "uart_block_received_count()"
 */
void uart_block_receive(uint8_t *dest, uint8_t size, void (*callback)(void));
void uart_block_receive_end(void);
int  uart_block_received_count(void);

/**
 * Send from UART a block of data located at "src" of length "len".
 * "callback" be invoked upon completion. Can be interrupted by
 * calling "uart_block_send_end()"
 */
void uart_block_send(uint8_t *src, uint8_t len, void (*callback)(void));
void uart_block_send_end(void);

/**
 * Write a character to UART. Blocks until any ongoing block
 * transfer completes.
 */
#ifdef MSPGCC
int putchar(int c);
#endif