#include <stdint.h>
#include <avr/io.h>

#include "rf.h"
#include "debug.h"

static void rf_set_state(uint8_t state) {
	while((TRX_STATUS & 0x1F) == 0x1F);

	TRX_STATE = state;

	while((TRX_STATUS & 0x1F) == 0x1F);

	assert((TRX_STATUS & 0x1F) == state);
}

void rf_init(void) {
	rf_set_state(CMD_PLL_ON);
}

void rf_receive(void) {
	rf_set_state(CMD_RX_ON);
}

void rf_transmit(uint8_t *data, uint8_t len) {
	assert(len < 128);

	uint8_t *buf = (uint8_t*)&TRXFBST;

	*buf++ = len;

	while(len--)
		*buf++ = *data++;

	rf_set_state(CMD_TX_START);
}

void rf_set_channel(uint8_t channel) {
	uint8_t phy_cc_cca = PHY_CC_CCA;

	phy_cc_cca &= ~0x1F;
	phy_cc_cca |= channel & 0x1F;

	PHY_CC_CCA = phy_cc_cca;
}
