#ifndef __WIRELESS_H
#define __WIRELESS_H

void WirelessUsart_Init(void);
void WirelessGPIO_Config(void);
void SetMode(u8 mode);
void WirelessSend(u8 data);
#endif


