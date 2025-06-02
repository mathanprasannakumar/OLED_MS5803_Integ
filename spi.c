#include <msp430.h>
#include "spi.h"


void spi_init(void) {
    // SPI configuration for DISPLAY - USART0
    P3DIR |= BIT0 | BIT1 | BIT3 | BIT6 | BIT7; // CS, MOSI, CLK, RES, DC as output
    P3SEL |= BIT1 | BIT2 | BIT3;               // Enable SPI function on MOSI, MISO, CLK
//    P3DIR &= ~BIT2;                            // MISO as input

    // Pull default values high
    P3OUT |= BIT0 | BIT6 | BIT7;

    // Configure SPI (USART0 in SPI master mode)
    U0CTL |= SWRST;                          // Reset SPI state machine
    U0CTL |= CHAR + SYNC + MM;               // 8-bit, sync, master mode
    U0TCTL = SSEL1 + STC;             // Clock polarity, SMCLK, 3-wire

//    U0BR0 = 2;                               // Baud rate = SMCLK / 2
//    U0BR1 = 0;

    ME1 |= USPIE0;                           // Enable SPI
    U0CTL &= ~SWRST;                         // Enable SPI

    // SPI Configuration for MS5803 - USART1
    P5SEL |= BIT1 | BIT2 | BIT3;  // SIMO, SOMI, UCLK
    P5DIR |= BIT0 | BIT1 | BIT3;  // CS,SIMO, UCLK as output
    P5DIR &= ~BIT2;               // SOMI as input
    P5OUT |= BIT0;  // CS high

    U1CTL = SWRST;                // Software reset
    U1CTL |= CHAR + SYNC + MM;   // 8-bit, SPI master
    U1TCTL = STC + SSEL1;        // 3-pin, SMCLK
    // TODO : why divide as max requirement is 20mhz on the sensor side ?
//
//    U1BR0 = 2;                   // Clock divider (adjust as needed)
//    U1BR1 = 0;

    ME2 |= USPIE1;               // Enable SPI
    U1CTL &= ~SWRST;             // Release reset

}


void spi_send(uint8_t data,uint8_t usart_flag) {
    //DOUBT: Here there is no MISO line is connected to display , but during transmission from
    // master to slave , interrupt for URXIFG0 is being set.

    if(USART1 & usart_flag) // use USART1
    {
        while (!(IFG2 & UTXIFG1));
        U1TXBUF = data;
        while (!(IFG2 & URXIFG1));
        (void)U1RXBUF; // Dummy read
    }
    else                      // use USART0
    {
        while (!(IFG1 & UTXIFG0));  // Wait for TX buffer ready
        U0TXBUF = data;
        while (!(IFG1 & URXIFG0));  // Wait for TX complete
        volatile uint8_t dummy = U0RXBUF; // Clear RX flag
    }
}

uint8_t spi_recv(uint8_t usart_flag)
{
    if(USART1 & usart_flag)
    {
        while (!(IFG2 & UTXIFG1));
        U1TXBUF = 0x00;
        while (!(IFG2 & URXIFG1)); // interrupt flag set when the USART1 receives a byte
        return U1RXBUF;
    }
    else
    {
        while (!(IFG1 & UTXIFG0));
        U0TXBUF = 0x00;
        while (!(IFG1 & URXIFG0)); // interrupt flag set when the USART0 receives a byte
        return U0RXBUF;
    }

}
