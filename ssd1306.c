#include "ssd1306.h"
#include "spi.h"
#include "delay.h"

// OLED control pin macros
#define OLED_CS_LOW    (P3OUT &= ~BIT0)
#define OLED_CS_HIGH   (P3OUT |=  BIT0)
#define DC_CMD    (P3OUT &= ~BIT7)  // Command mode
#define DC_DATA   (P3OUT |=  BIT7)  // Data mode
#define RES_LOW   (P3OUT &= ~BIT6)
#define RES_HIGH  (P3OUT |=  BIT6)

// font map
const uint8_t font5x7_digits[10][5] = {
    {0x3E, 0x51, 0x49, 0x45, 0x3E}, // '0'
    {0x00, 0x42, 0x7F, 0x40, 0x00}, // '1'
    {0x42, 0x61, 0x51, 0x49, 0x46}, // '2'
    {0x21, 0x41, 0x45, 0x4B, 0x31}, // '3'
    {0x18, 0x14, 0x12, 0x7F, 0x10}, // '4'
    {0x27, 0x45, 0x45, 0x45, 0x39}, // '5'
    {0x3C, 0x4A, 0x49, 0x49, 0x30}, // '6'
    {0x01, 0x71, 0x09, 0x05, 0x03}, // '7'
    {0x36, 0x49, 0x49, 0x49, 0x36}, // '8'
    {0x06, 0x49, 0x49, 0x29, 0x1E}  // '9'
};

void write_command(uint8_t cmd) {
    OLED_CS_LOW;
    DC_CMD;
    spi_send(cmd,USART0);
    OLED_CS_HIGH;
}


void write_data(uint8_t data) {
    OLED_CS_LOW;
    DC_DATA;
    spi_send(data,USART0);
    OLED_CS_HIGH;
}

void oled_reset(void) {
    // As per ssd1306, min delay of 3 micro seconds needed after RES_LOW ( Page:27, Section: 8.9 )
    RES_LOW;
    DELAY_MICRO_SECS(5);
    RES_HIGH;
    DELAY_MICRO_SECS(5);
}

void oled_init(void) {
    oled_reset();
    // As per 1306 , min delay of 100 ms needed after reset so that SEG and COM will be active
    DELAY_MILLI_SECS(100);

    write_command(0xAE); // Display OFF
    write_command(0xD5); write_command(0x80); // Set clock
    write_command(0xA8); write_command(0x3F); // Multiplex = 64
    write_command(0xD3); write_command(0x00); // Display offset = 0
    write_command(0x40); // Start line = 0
    write_command(0x8D); write_command(0x14); // Charge pump ON
    write_command(0x20); write_command(0x00); // Horizontal addressing
    write_command(0xA1); // Segment remap (horizontal flip)
    write_command(0xC8); // COM scan direction (vertical flip)
    write_command(0xDA); write_command(0x12); // COM pins config
    write_command(0x81); write_command(0x7F); // Contrast
    write_command(0xD9); write_command(0xF1); // Pre-charge
    write_command(0xDB); write_command(0x40); // VCOM deselect
    write_command(0xA4); // Resume RAM display
    write_command(0xA6); // Normal display mode
    write_command(0xAF); // Display ON
}

void oled_clear(void) {
        uint8_t page;
        uint8_t col;

        // As in horizontal addressing mode , column address pointer incremented to 1 when read/write at the current address and reset to 0 when column pointer reaches the end
        // similarly , page address pointer is incremented to 1 when the whole page is read/write and reset to 0 when it reaches the end.
        write_command(0xB0  );  // Set page address
        write_command(0x00);         // Lower column start
        write_command(0x10);         // Higher column start
        for (page = 0; page < 8; page++) {
            for (col= 0; col < 128; col++) {
                write_data(0x00);        // All pixels OFF
            }

        }
}

void oled_test_pattern(void) {
    uint8_t page;
    uint8_t col;

    write_command(0xB0);
    write_command(0x00);
    write_command(0x10);

    for (page = 0; page < 8; page++) {

        for (col = 0; col < 128; col++) {
            write_data((col % 10) ? 0xFF : 0x00); // Stripe pattern // here observed 1 for black and 0 for light
        }
    }
}

void oled_draw_digit(uint8_t digit, uint8_t page, uint8_t column) {
    uint8_t i;
    if (digit > 9) return;

    // Set page
    write_command(0xB0 + page);

    // Set column address
    write_command(column & 0x0F);             // Lower nibble
    write_command(0x10 | ((column >> 4) & 0x0F)); // Upper nibble

    // Send font bytes
    for (i = 0; i < 5; i++) {
        write_data(font5x7_digits[digit][i]);
    }

    write_data(0x00); // 1-column space
    write_data(0x00); // 1-column space
}

void convert_digit_to_array(int data , uint8_t * digits,uint8_t len)
{
      int temp = data;
      int count = 0;

      // Count digits (we'll reverse later)
      while (temp > 0) {
          digits[count++] = temp % 10;
          temp /= 10;
      }

      while(count <= (len-1))
      {
          digits[count++] = 0;
      }

      int i ;
      // Now reverse the array to get digits in correct order
      for (i = 0; i < count / 2; i++) {
          int t = digits[i];
          digits[i] = digits[count - 1 - i];
          digits[count - 1 - i] = t;
      }

}


void oled_put_data(float data,uint8_t page , uint8_t start_col)
{
    uint8_t int_digit[4];
    uint8_t decimal_digit[2];


    int int_part = (int)data;
    int decimal_part = (int)((data - int_part)*100);

    if(decimal_part < 0 ) decimal_part = -decimal_part;

    convert_digit_to_array(int_part,int_digit,4);
    convert_digit_to_array(decimal_part,decimal_digit,2);

    uint8_t i;
    for ( i = 0;i<4;i++)
    {
        oled_draw_digit(int_digit[i],page,start_col+(i*7));
    }

    write_data(0xFF);
    write_data(0x00);
    start_col = start_col + 30;

    for (i = 0;i<2;i++)
    {
       oled_draw_digit(decimal_digit[i],page,start_col+(i*7));
    }

}

