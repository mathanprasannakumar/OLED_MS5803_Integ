#include <msp430.h>
#include "spi.h"
#include "ssd1306.h"
#include "delay.h"
#include "ms5803.h"



// TODO :
/*                                                                              Status
 * 1) Investigate the timer module for perfect delay loop                        done
 * 2) Work on the recommended power on and off sequence of  display
 * 3) fix exact time delay where time delay are required
 * 4) Try out bigger font map
 * 5) Display showcases two color separately , find way to configure it
 * 6)

 */

void init_xt2_clock()
{
   BCSCTL1 &= ~XT2OFF;             // Enable XT2
   do {
     IFG1 &= ~OFIFG;             // Clear oscillator fault flag
     __delay_cycles(1000);       // Delay to allow oscillator to stabilize
   }
   while (IFG1 & OFIFG);         // Check if fault flag is set

   BCSCTL2 = SELM_2 | SELS;        // Set MCLK and SMCLK to XT2
}



int main(void) {
    WDTCTL = WDTPW | WDTHOLD; // Stop watchdog

    init_xt2_clock();

    spi_init();       // Setup SPI + GPIO
    reset_ms5803();
    read_prom();

    oled_init();      // Init display
    oled_clear();     // Clear screen
//    oled_test_pattern(); // Show pattern

    while (1)
    {
       start_conversion();

       float temperature, pressure;
       calculate_temperature_pressure(&temperature, &pressure);


       oled_put_data(pressure,4,52);
       oled_put_data(temperature,6,52);

//        oled_draw_digit(0,4,52);
//        oled_draw_digit(0,4,59);
//        oled_draw_digit(0,4,66);
//        oled_draw_digit(0,4,73);

//        oled_draw_digit(1,5,52);
//        oled_draw_digit(1,5,59);
//        oled_draw_digit(1,5,66);
//        oled_draw_digit(1,5,73);

        timerA_delay(10,DURATION_MILLI);
    }

}
