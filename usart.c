#include <stdint.h>
#include <avr/io.h>
#include <avr/pgmspace.h>

#include "usart.h"

#define BAUD 38400
#include <util/setbaud.h>

void usart_init(void) {
	UBRR1H = UBRRH_VALUE;
	UBRR1L = UBRRL_VALUE;

	UCSR1A = (USE_2X << U2X1);
	UCSR1B = (1 << RXEN1) | (1 << TXEN1);
}

void usart_putc(uint8_t c) {
	while(!(UCSR1A & (1 << UDRE1)));

	UDR1 = c;
}

uint8_t usart_getc(void) {
	while(!(UCSR1A & (1 << RXC1)));

	return UDR1;
}

void usart_puts(const char *s) {
	uint8_t c;

	while((c = *s++) != 0)
		usart_putc(c);
}

static void usart_nibble(uint8_t x) {
	x &= 0x0F;

	if(x >= 10)
		usart_putc(x - 10 + 'a');
	else
		usart_putc(x + '0');
}

void usart_hex(uint8_t x) {
	usart_nibble(x >> 4);
	usart_nibble(x & 0x0F);
}

uint8_t usart_read_hex(void) {
	uint8_t c, out;

	c = usart_getc() | 0x20;
	out = (c >= 'a') ? (c - 'a' + 10) : (c - '0');

	out <<= 4;
	c = usart_getc() | 0x20;

	out |= (c >= 'a') ? (c - 'a' + 10) : (c - '0');

	return out;
}
