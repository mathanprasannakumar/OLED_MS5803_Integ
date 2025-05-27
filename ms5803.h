#ifndef MS5803_H
#define MS5803_H


void reset_ms5803(void);
void read_prom(void);
void start_conversion(void);
void calculate_temperature_pressure(float *temperature, float *pressure);

#endif
