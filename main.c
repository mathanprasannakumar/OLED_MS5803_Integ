#include <msp430.h>
#include "spi.h"
#include "ssd1306.h"
#include "delay.h"
#include "ms5803.h"




// TODO :
/*
 * 1) Change the structure of the code for sequence
 * 2) Increase the timer for consequent press
 * 3) Increase the font size
 * 4) Pressure -> altitude calculation
 * 5)
 */

uint8_t button_counter= 0;

uint16_t led_duration = 1500; // ms


#define LED_PIN BIT3
#define BUTTON_PIN BIT2

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

inline void timerB_start(void)
{
    led_duration = 1000;

    TBCTL = MC_0;
    // Use SMCLK, Up mode, Clear TAR
    TBCTL = TBSSEL_2 | MC_1 | TBCLR;
    TBCCTL0 = CCIE;
    TBCCR0= XT2CLK_CYCLES_PER_MILLI_SEC -1;
}

inline void timerB_stop(void)
{
    TBCTL = MC_0;
}

inline void led_start(void)
{
    P1OUT |=LED_PIN;
}

inline void led_stop(void)
{
    P1OUT &=~LED_PIN;
}


#pragma vector=TIMERB0_VECTOR
__interrupt void TimerBInterrupt(void)
{
    led_duration--;

    if(!led_duration)
    {
        button_counter = 0;
        timerB_stop();
        led_stop();
    }
    // duration end

}

#pragma vector=PORT1_VECTOR
__interrupt void Port1(void)
{
    if(P1IFG & BIT2)
    {
        // when the button pressed for the first time or button pressed after 1 sec of previous press
        if(button_counter==0)
        {
            button_counter++;
            DELAY_SECS(1); // no led blink for 1 sec

            led_start(); // led blink
            timerB_start(); // starting the 1 sec timer here

        }
        else // when button pressed within 1sec of previous press
        {
            button_counter++;
            led_stop(); // after 2-4 press within 1 sec , stopping the led
            timerB_stop(); // stopping the timer as , from this instant timer should be started

            if(button_counter == 4)
            {
                button_counter = 0;
                P1IE &=~BUTTON_PIN; // disable button interrupt
                P1IFG &=~BUTTON_PIN;

                __low_power_mode_off_on_exit();

                return;
            }

            DELAY_SECS(1);

            led_start(); // led blink for the current press
            timerB_start(); // starting 1 sec timer here
        }

    }
    P1IFG &=~BIT2;
}

void led_init(void)
{
    P1DIR |= LED_PIN;
    P1OUT &=~LED_PIN;
}

void button_init(void)
{
    P1DIR&=~BUTTON_PIN;

    P1IFG &=~BUTTON_PIN;
    P1IES |=BUTTON_PIN;
    P1IE |=BUTTON_PIN;

}

int main(void) {
    WDTCTL = WDTPW | WDTHOLD; // Stop watchdog

    init_xt2_clock();
    led_init();
    __enable_interrupt();

    while(1)
    {
     button_init();

    __low_power_mode_0();

    spi_init();       // Setup SPI + GPIO
    reset_ms5803();
    read_prom();

    oled_init();      // Init display
//    oled_clear();


// calibration loop

    uint16_t duration = 10; // sec

    uint16_t avg = 0;
    float sum = 0;
    uint8_t n_digits = 0; // num of digits drawn on the display

    uint8_t curr_page;
    uint8_t curr_col;
    uint8_t mid_column = 64;

    while (duration)
    {
       start_conversion();

       float temperature;
       float pressure;

       calculate_temperature_pressure(&temperature,&pressure);
       sum+=pressure;

       //TODO: here what if for single digit , i am drawing digit + empty 5 pixel

       if(duration > 9)
       {   // curr_page and curr_col gets updated based on the n_digts
           oled_draw_digits(duration,&curr_page,&curr_col,&n_digits);
       }
       else
       {
           if(n_digits==2)
           {
               oled_clear_digits(&curr_page,&curr_col,&n_digits,1);
           }
         oled_draw_digit(duration,curr_page,curr_col);
       }

        timerA_delay(500,DURATION_MILLI);
        duration--;
    }

    oled_clear_digit(curr_page,curr_col);
    n_digits--;

    /*
     * 1) show pressure data
     * 2) show error code : A6E
     */
      // pressure data vis

    avg = (int)(sum/10);
    oled_draw_digits(avg,&curr_page,&curr_col,&n_digits);
    timerA_delay(1,DURATION_SEC);

    // clear pressure data
    oled_clear_digits(&curr_page,&curr_col,&n_digits,n_digits);
    timerA_delay(500,DURATION_MILLI);

    // error vis
    // TODO: only show error data when crossing the maintanence date
    curr_col = mid_column - 7;
    oled_draw_char(0,curr_page,curr_col);
    oled_draw_digit(6,curr_page,curr_col+(1*7));
    oled_draw_char(4,curr_page,curr_col+(2*7));
    n_digits += 3;

    uint8_t user_edit_mode = 0;
    uint8_t stat_mode = 0;
    uint8_t button_hold=0;

    duration = 400;
    while(duration)
    {
        if(P1IN & BIT2)
        {
           button_hold = 1;
           user_edit_mode = 1;
        }
        else
        {
            if(button_hold)
            {
               user_edit_mode = 0;
               break;
            }
        }
        timerA_delay(10,DURATION_MILLI);
        duration--;
    }

    while(duration)
    {
        timerA_delay(10,DURATION_MILLI);
        duration--;
    }


    // clear error data
    oled_clear_digits(&curr_page,&curr_col,&n_digits,2); // clearing the error code A6
    // now curr_col will be pointing to the e in display
    oled_draw_digit(0,curr_page,curr_col);


    if(user_edit_mode)
    {
        curr_col = mid_column - (2*7); // for four zeros
        oled_draw_digit(0,curr_page,curr_col); // first zero
        oled_draw_digit(0,curr_page,curr_col+(1*7)); // second zero
        oled_draw_digit(0,curr_page,curr_col+(2*7)); // third zero
        n_digits +=3;

        duration = 100;

        uint8_t press_ref[4] = {0,1,1,1};

        uint8_t index = 0;
        // first digit
        while(P1IN&BIT2)
        {

            if(index == 100) // each second
            {
                if(press_ref[0] == 0)
                {
                    oled_draw_digit(1,curr_page,curr_col);
                    press_ref[0] = 1;
                }
                else
                {
                    oled_draw_digit(0,curr_page,curr_col);
                    press_ref[0] = 0;
                }
                index = 0;
            }

            timerA_delay(10,DURATION_MILLI);
            index++;
         }

        oled_clear_digit(curr_page,curr_col+(1*7));
        oled_clear_digit(curr_page,curr_col+(2*7));
        oled_clear_digit(curr_page,curr_col+(3*7));
        n_digits-=3;
        curr_col +=7;

        while(!(P1IN&BIT2)); // wait for next hold

        // second digit
        index = 0;
        uint8_t num =1;
        while(P1IN&BIT2)
        {
            if(index == 100)
            {
                if(num == 10)
                {
                    num = 0;
                    oled_draw_digit(num,curr_page,curr_col);
                    press_ref[1] = num;
                }
                else
                {

                    oled_draw_digit(num,curr_page,curr_col);
                    press_ref[1] = num;
                    num++;
                }
                index = 0;

            }

            timerA_delay(10,DURATION_MILLI);
            index++;
         }

        n_digits++;
        curr_col+=7;

        while(!(P1IN&BIT2));

        // third digit
        index = 0;
        num =1;
        while(P1IN&BIT2)
        {
            if(index == 100)
            {
                if(num == 10)
                {
                    num = 0;
                    oled_draw_digit(num,curr_page,curr_col);
                    press_ref[2] = num;
                 }
                 else
                 {

                    oled_draw_digit(num,curr_page,curr_col);
                    press_ref[2] = num;
                    num++;
                 }
                index = 0;
             }

             timerA_delay(10,DURATION_MILLI);
             index++;
         }
        n_digits++;
        curr_col+=7;

        while(!(P1IN&BIT2));

        // fourth digit
        index = 0;
        num =1;
        while(P1IN&BIT2)
        {
            if(index == 100)
            {
                if(num == 10)
                {
                    num = 0;
                    oled_draw_digit(num,curr_page,curr_col);
                    press_ref[3] = num;
                 }
                 else
                 {

                    oled_draw_digit(num,curr_page,curr_col);
                    press_ref[3] = num;
                    num++;
                 }
                 index = 0;
             }
            timerA_delay(10,DURATION_MILLI);
            index++;
         }
         n_digits++;
         timerA_delay(1,DURATION_SEC);
         led_start();
         timerA_delay(4,DURATION_SEC);
         led_stop();

     }
     else
     {
        /// stat mode identification , after 0 is found on the display , user has to hold for a duration of 2 sec
         button_hold = 0;
         duration = 200;
         while(duration)
         {
             if(P1IN & BIT2)
             {
               button_hold = 1;
               stat_mode = 1;
             }
             else
             {
                if(button_hold)
                {
                   stat_mode = 0;
                   break;
                }
             }
             timerA_delay(10,DURATION_MILLI);
             duration--;
          }

          while(duration)
          {
              timerA_delay(10,DURATION_MILLI);
              duration--;
          }
     }
     if(stat_mode)
     {

          uint16_t index = 0;
         // as serial code shown as a 6 digit , placing col 3 digit back from the mid col
         curr_col = mid_column -(3*7);
         oled_draw_char(17,curr_page,curr_col);
         oled_draw_char(0,curr_page,curr_col+(1*7));
         oled_draw_char(8,curr_page,curr_col+(2*7));
         oled_draw_digit(1,curr_page,curr_col+(3*7));
         oled_draw_digit(2,curr_page,curr_col+(4*7));
         oled_draw_digit(3,curr_page,curr_col+(5*7));
         n_digits = 6;

         while(P1IN&BIT2)
         {
              if(index == 500)
              {
                   break;
              }

               timerA_delay(10,DURATION_MILLI);
               index++;
          }
         oled_clear_digits(&curr_page,&curr_col,&n_digits,6);
         curr_col = mid_column+7;
         oled_draw_digit(0,curr_page,curr_col);
         n_digits=1;
      }

     button_init();

     __low_power_mode_0();

     // clear zero
     if(user_edit_mode)
     {
        curr_col = mid_column - (2*7);
        oled_clear_digits(&curr_page,&curr_col,&n_digits,n_digits);
     }
     else
     {
        oled_clear_digit(curr_page,curr_col);
        n_digits =0;
     }

     oled_shutdown();
     timerA_delay(1,DURATION_SEC);

    }


}
