#ifndef RF_H
#define RF_H

void rf_init(void);
void rf_receive(void);
void rf_transmit(uint8_t *data, uint8_t len);
void rf_set_channel(uint8_t channel);

#endif
