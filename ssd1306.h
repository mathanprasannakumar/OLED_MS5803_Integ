#ifndef SSD1306_H
#define SSD1306_H

#include <stdint.h>
#include <msp430.h>

void oled_init(void);
void oled_clear(void);
void oled_draw_digit(uint8_t digit, uint8_t page, uint8_t column);
void oled_test_pattern(void);
//void oled_put_data(float data,uint8_t page , uint8_t start_col);
void oled_shutdown(void);
void oled_draw_large_digit(uint8_t digit, uint8_t start_page, uint8_t start_column);
void oled_clear_digit(uint8_t page, uint8_t column);
void convert_digit_to_array(uint16_t data , uint8_t * digits,uint8_t width,uint8_t *n_digits);
void oled_draw_digits(uint16_t digit , uint8_t *page , uint8_t*column,uint8_t* n_digits);
void oled_draw_char(uint8_t char_pos,uint8_t page, uint8_t column);
void oled_clear_digits(uint8_t *page, uint8_t *column,uint8_t *n_digits,uint8_t clear_count);
#endif
