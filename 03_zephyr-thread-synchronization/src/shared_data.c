#include "shared_data.h"

/* Definicija dijeljenih podataka */
struct sensor_data shared_sensors;

void sensor_data_init(void) {
    k_mutex_init(&shared_sensors.lock);
    shared_sensors.temperature = 25.0f;
    shared_sensors.humidity = 50;
}

void sensor_data_set_temp(float temp) {
    k_mutex_lock(&shared_sensors.lock, K_FOREVER);
    shared_sensors.temperature = temp;
    k_mutex_unlock(&shared_sensors.lock);
}

void sensor_data_set_humidity(int humidity) {
    k_mutex_lock(&shared_sensors.lock, K_FOREVER);
    shared_sensors.humidity = humidity;
    k_mutex_unlock(&shared_sensors.lock);
}

float sensor_data_get_temp(void) {
    float temp;
    k_mutex_lock(&shared_sensors.lock, K_FOREVER);
    temp = shared_sensors.temperature;
    k_mutex_unlock(&shared_sensors.lock);
    return temp;
}

int sensor_data_get_humidity(void) {
    int humidity;
    k_mutex_lock(&shared_sensors.lock, K_FOREVER);
    humidity = shared_sensors.humidity;
    k_mutex_unlock(&shared_sensors.lock);
    return humidity;
}
