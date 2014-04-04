#ifndef DEBUG_H
#define DEBUG_H

#include <avr/pgmspace.h>

#include "usart.h"

#define assert(x) do {\
	if(!(x)) {\
		usart_puts("M assertion \"" #x "\" failed");\
		while(1);\
	}\
} while(0)

#endif
