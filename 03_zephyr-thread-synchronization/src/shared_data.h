#ifndef SHARED_DATA_H
#define SHARED_DATA_H

#include <zephyr/kernel.h>

/* Struktura za dijeljene senzorske podatke */
struct sensor_data {
    float temperature;
    int humidity;
    struct k_mutex lock;
};

/* Globalna varijabla dostupna svim modulima */
extern struct sensor_data shared_sensors;

/* Funkcije za siguran pristup podacima */
void sensor_data_init(void);
void sensor_data_set_temp(float temp);
void sensor_data_set_humidity(int humidity);
float sensor_data_get_temp(void);
int sensor_data_get_humidity(void);

#endif
