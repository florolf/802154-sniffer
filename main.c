#include <stdint.h>
#include <string.h>

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "debug.h"
#include "led.h"
#include "usart.h"
#include "rf.h"

volatile bool status = false;

ISR(TRX24_RX_END_vect) {
	status = !status;
	led_set(status);

	usart_puts("D ");

	usart_putc((PHY_RSSI & 0x80) ? '1':'0');

	usart_putc(' ');

	uint8_t len = TST_RX_LENGTH;
	usart_hex(len);

	usart_putc(' ');

	uint8_t *buf = (uint8_t*)&TRXFBST;
	while(len--) {
		uint8_t data = *buf++;
		usart_hex(data);
	}

	usart_putc(' ');

	usart_hex(*buf);
	usart_puts("\n");
}

volatile bool running = false;
uint32_t remaining_cnt;
uint8_t min, max;
uint32_t sum;

#define MEASURE_CNT 100000

ISR(TRX24_CCA_ED_DONE_vect) {
	uint8_t val = PHY_ED_LEVEL;

	if(val == 0xFF)
		return;

	if(val < min)
		min = val;

	if(val > max)
		max = val;

	sum += val;

	if(running) {
		remaining_cnt--;

		if(!remaining_cnt)
			running = false;
		else
			PHY_ED_LEVEL = 1;
	}
}

static void cmd_set_channel(void) {
	uint8_t channel = usart_read_hex();

	rf_set_channel(channel);
}

static void energy_scan(void) {
	if(running) {
		usart_puts("E\n");
		return;
	}

	min = 0xFF;
	max = 0;
	sum = 0;

	IRQ_MASK |= 1 << CCA_ED_DONE_EN;
	remaining_cnt = MEASURE_CNT;
	running = true;
	PHY_ED_LEVEL = 1;

	while(running);
	IRQ_MASK &= ~(1 << CCA_ED_DONE_EN);

	usart_puts("O ");
	usart_hex(min); usart_putc(' ');
	usart_hex(max); usart_putc(' ');

	uint8_t avg = sum / MEASURE_CNT;
	usart_hex(avg); usart_putc('\n');
}

int main(void) {
	usart_init();
	usart_puts("M starting up\n");

	led_init();
	rf_init();

	IRQ_MASK |= 1 << RX_END_EN;

	sei();

	rf_receive();
	while(1) {
		switch(usart_getc()) {
			case 'c': // set channel
				cmd_set_channel();
				break;
			case 'p': // ping
				usart_puts("O\n");
				break;
			case 'e': // energy scan
				energy_scan();
				break;
		}
	}
}
