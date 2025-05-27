#ifndef SSD1306_H
#define SSD1306_H

#include <stdint.h>
#include <msp430.h>

void oled_init(void);
void oled_clear(void);
void oled_draw_digit(uint8_t digit, uint8_t page, uint8_t column);
void oled_test_pattern(void);
void oled_put_data(float data,uint8_t page , uint8_t start_col);

#endif
