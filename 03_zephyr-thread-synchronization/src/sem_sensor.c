#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/random/random.h>
#include "shared_data.h"

#define WORKER_STACK_SIZE 1024
#define WORKER_PRIORITY 6
#define HUMIDITY_SENSOR_PERIOD K_SECONDS(5)

K_SEM_DEFINE(data_available, 0, 1);

static struct {
    float temperature;
    k_spinlock_key_t key;
} sensor_buffer = {
    .temperature = 0.0f,
    .key = { 0 }
};

static struct k_spinlock data_lock;

/* Timer za simulaciju humidity senzora */
static void humidity_sensor_simulation(struct k_timer *timer) {
    static int humidity = 50;
    humidity += (sys_rand32_get() % 11 - 5);
    if(humidity < 30) humidity = 30;
    if(humidity > 80) humidity = 80;
    
    sensor_data_set_humidity(humidity);
    uint32_t uptime = k_uptime_get_32();
    printk("[%6d.%03d] [HUMIDITY-SENSOR] Nova vlažnost: %d%%\n", 
           uptime/1000, uptime%1000, humidity);
}

K_TIMER_DEFINE(humidity_timer, humidity_sensor_simulation, NULL);

static void sensor_isr_simulation(struct k_timer *timer) {
    k_spinlock_key_t key = k_spin_lock(&data_lock);
    sensor_buffer.temperature = 20.0f + (sys_rand32_get() % 150) * 0.1f;
    k_spin_unlock(&data_lock, key);
    
    uint32_t uptime = k_uptime_get_32();
    printk("[%6d.%03d] [SEMAFOR-ISR] Nova temperatura: %.1f°C - Signal poslat!\n", 
           uptime/1000, uptime%1000, sensor_buffer.temperature);
    k_sem_give(&data_available);
}

K_TIMER_DEFINE(sensor_timer, sensor_isr_simulation, NULL);

static void sensor_worker_thread(void *p1, void *p2, void *p3) {
    float local_temp;
    
    while(1) {
        k_sem_take(&data_available, K_FOREVER);
        
        k_spinlock_key_t key = k_spin_lock(&data_lock);
        local_temp = sensor_buffer.temperature;
        k_spin_unlock(&data_lock, key);
        
        /* Ažuriraj dijeljene podatke */
        sensor_data_set_temp(local_temp);
        
        k_sleep(K_MSEC(200));
        
        if(local_temp > 30.0f) {
            uint32_t uptime = k_uptime_get_32();
            printk("[%6d.%03d] [SEMAFOR-WORKER] >>> UPOZORENJE: Visoka temperatura: %.1f°C\n", 
                   uptime/1000, uptime%1000, local_temp);
        } else if(local_temp < 22.0f) {
            uint32_t uptime = k_uptime_get_32();
            printk("[%6d.%03d] [SEMAFOR-WORKER] >>> INFO: Niska temperatura: %.1f°C\n", 
                   uptime/1000, uptime%1000, local_temp);
        } else {
            uint32_t uptime = k_uptime_get_32();
            printk("[%6d.%03d] [SEMAFOR-WORKER] >>> INFO: Normalna temperatura: %.1f°C\n", 
                   uptime/1000, uptime%1000, local_temp);
        }
    }
}

K_THREAD_DEFINE(worker_tid, WORKER_STACK_SIZE, sensor_worker_thread,
                NULL, NULL, NULL, WORKER_PRIORITY, 0, 0);

void sem_sensor_init(void) {
    printk("SEMAFOR ZADATAK: Pokrenut\n");
    k_timer_start(&sensor_timer, K_MSEC(1000), K_MSEC(800));
    k_timer_start(&humidity_timer, K_SECONDS(2), HUMIDITY_SENSOR_PERIOD);
}
