#ifndef SPI_H
#define SPI_H
#include <stdint.h>

#define USART0                   0x00
#define USART1                   0x01

void spi_init(void);
void spi_send(uint8_t data,uint8_t usart_flag);
uint8_t spi_recv(uint8_t usart_flag);

#endif
