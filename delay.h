#ifndef DELAY_H
#define DELAY_H

#include <stdint.h>

#define DURATION_SEC    0x02
#define DURATION_MILLI  0x01
#define DURATION_MICRO  0x00

#define XT2CLK_FREQ 8000000UL
#define XT2CLK_CYCLES_PER_MILLI_SEC (XT2CLK_FREQ/1000)
#define XT2CLK_CYCLES_PER_MICRO_SEC (XT2CLK_FREQ/1000000UL)

#define DELAY_MILLI_SECS(x) __delay_cycles((x)*XT2CLK_CYCLES_PER_MILLI_SEC)
#define DELAY_MICRO_SECS(x) __delay_cycles((x)*XT2CLK_CYCLES_PER_MICRO_SEC)
#define DELAY_SECS(x) __delay_cycles((x)*XT2CLK_FREQ)



void timerA_delay(unsigned int duration, uint8_t duration_unit);

#endif
