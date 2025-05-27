#include "spi.h"
#include "delay.h"
#include <msp430.h>
#include "ms5803.h"

#define MS5803_CS_HIGH (P5OUT |= BIT0)
#define MS5803_CS_LOW  (P5OUT &= ~BIT0)
#define MS5803_RESET_COMMAND     0x1E
#define MS5803_CONVERT_D1_4096   0x48
#define MS5803_CONVERT_D2_4096   0x58
#define MS5803_ADC_READ          0x00
#define MS5803_PROM_BASE_ADDR    0xA0


uint16_t prom[8];
volatile uint32_t D1;
volatile uint32_t D2;


void reset_ms5803(void)
{
    MS5803_CS_LOW;
    // as per datasheet, have to wait for min of 2.88 ms after reset
    spi_send(MS5803_RESET_COMMAND,USART1);
    DELAY_MILLI_SECS(5);
    MS5803_CS_HIGH;
}

void read_prom(void)
{
    uint8_t i;
    for (i = 0; i < 8; i++) {

         MS5803_CS_LOW;
         // PROM read address command
         spi_send(MS5803_PROM_BASE_ADDR + (i * 2),USART1);
         // 16 bit value returned
         uint8_t msb = spi_recv(USART1);
         uint8_t lsb = spi_recv(USART1);
         MS5803_CS_HIGH;

         prom[i] = (msb << 8) | lsb;
         DELAY_MILLI_SECS(5);
     }
}


// TODO : how to get to know the succesfull conversion
uint32_t read_adc(void)
{
   uint32_t adc = 0;

   MS5803_CS_LOW;

   spi_send(MS5803_ADC_READ,USART1);

   adc = ((uint32_t)spi_recv(USART1)) << 16;
   adc |= ((uint32_t)spi_recv(USART1)) << 8;
   adc |= spi_recv(USART1);

   MS5803_CS_HIGH;

   return adc;

}

void start_conversion(void)
{
    /*
     * After the conversion, using ADC read command the result is clocked out with the MSB first.
     * If the conversion is not executed before the ADC read command, or the ADC read command is repeated,
     * it will give 0 as the output result.
     * If the ADC read command is sent during conversion the result will be 0,
     * the conversion will not stop and
     * the final result will be wrong.
     */
    MS5803_CS_LOW;
    spi_send(MS5803_CONVERT_D1_4096,USART1);
    DELAY_MILLI_SECS(10);
    MS5803_CS_HIGH;
    D1 = read_adc();

    MS5803_CS_LOW;
    spi_send(MS5803_CONVERT_D2_4096,USART1);
    DELAY_MILLI_SECS(10);
    MS5803_CS_HIGH;
    D2 = read_adc();
}


void calculate_temperature_pressure(float *temperature, float *pressure)
{
    if( D1==0 || D2 == 0)
    {
        temperature = 0;
        pressure = 0;
        return;
    }

    //DOUBT:  why during this calculation D2 is zero( when build it with register level optimization)
    int32_t dT = D2 - ((uint32_t)prom[5] << 8);

    int32_t TEMP_VAR = ((int64_t)dT * prom[6])/ ((uint32_t)1<<23);

    int32_t TEMP = 2000 + TEMP_VAR;

    int64_t OFF = ((int64_t)prom[2] << 17) + (((int64_t)prom[4] * dT)/(1<<6));
    int64_t SENS = ((int64_t)prom[1] << 16) + (((int64_t)prom[3] * dT)/(1<<7));

    int32_t P = (((D1 * SENS)/((uint32_t)1 << 21) - OFF)) / ((uint32_t)1<<15);

    *temperature = TEMP / 100.0f;
    *pressure = P / 100.0f;
}




