#include <msp430.h>
#include "delay.h"




void timer_delay_loop(unsigned int duration)
{
    while (duration--)
    {
        // Wait for the flag to be set
        while (!(TACTL & TAIFG));
        // Clear the flag
        TACTL &= ~TAIFG;

    }
}

// Initialize Timer_A to create a delay in milliseconds
void timerA_delay(unsigned int duration, uint8_t duration_unit) {
    // Stop the timer
    TACTL = MC_0;

    // Use SMCLK, Up mode, Clear TAR
    TACTL = TASSEL_2 | MC_1 | TACLR;

    if(duration_unit & DURATION_MICRO) // 0 -> microseconds delay
    {
        TACCR0 = XT2CLK_CYCLES_PER_MICRO_SEC - 1; // cycles to increment the tar

    }
    else if(duration_unit & DURATION_MILLI)// 1 -> milliseconds delay
    {
        TACCR0 = XT2CLK_CYCLES_PER_MILLI_SEC -1;

    }
    else if(duration_unit & DURATION_SEC) // 2 -> seconds delay
    {
        // Here as TACCR0 is 16 bit cannot assign it to a value of DCO_FREQ(overflow happens)
        TACCR0 = XT2CLK_CYCLES_PER_MILLI_SEC -1;
        while(duration--)
        {
            timer_delay_loop(1000);
        }
        TACTL=MC_0;
        return;
    }
    else
    {
        TACTL = MC_0;
        return ;
    }

    timer_delay_loop(duration);
    // Stop the timer after delay
    TACTL = MC_0;
}
