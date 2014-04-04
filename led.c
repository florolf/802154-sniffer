#include <stdint.h>
#include <avr/io.h>

#include "led.h"

void led_init(void) {
	DDRF |= (1 << 2);

	led_set(false);
}

void led_set(bool state) {
	if(state)
		PORTF |= 1 << 2;
	else
		PORTF &= ~(1 << 2);
}
