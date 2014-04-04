#ifndef USART_H
#define USART_H

#include <stdint.h>

void usart_init(void);
void usart_putc(uint8_t c);
void usart_puts(const char *s);
void usart_puts_P(const char *s);
void usart_hex(uint8_t x);
uint8_t usart_getc(void);
uint8_t usart_read_hex(void);

#endif
