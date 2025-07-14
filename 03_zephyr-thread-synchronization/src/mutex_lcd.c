#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/random/random.h>
#include <string.h>
#include "shared_data.h"

#define LCD_STACK_SIZE 1024
#define LCD_PRIORITY 5

K_MUTEX_DEFINE(lcd_mutex);

static void lcd_print(const char *message) {
    uint32_t uptime = k_uptime_get_32();
    printk("[%6d.%03d] [MUTEX-LCD] ========== %s ==========\n", 
           uptime/1000, uptime%1000, message);
    k_sleep(K_MSEC(500));
}

static const char* get_system_status(void) {
    static const char* statuses[] = {"OK", "WARNING", "LOW BATTERY", "UPDATING"};
    return statuses[sys_rand32_get() % 4];
}

static void temperature_display_thread(void *p1, void *p2, void *p3) {
    char display_buffer[32];
    
    while(1) {
        k_mutex_lock(&lcd_mutex, K_FOREVER);
        float temp = sensor_data_get_temp();
        snprintf(display_buffer, sizeof(display_buffer), "Temp: %.1f°C", temp);
        lcd_print(display_buffer);
        k_mutex_unlock(&lcd_mutex);
        
        k_sleep(K_SECONDS(1.5));
    }
}

static void humidity_display_thread(void *p1, void *p2, void *p3) {
    char display_buffer[32];
    
    while(1) {
        k_mutex_lock(&lcd_mutex, K_FOREVER);
        int humidity = sensor_data_get_humidity();
        snprintf(display_buffer, sizeof(display_buffer), "Vlažnost: %d%%", humidity);
        lcd_print(display_buffer);
        k_mutex_unlock(&lcd_mutex);
        
        k_sleep(K_SECONDS(2.5));
    }
}

static void status_display_thread(void *p1, void *p2, void *p3) {
    char display_buffer[32];
    
    while(1) {
        k_mutex_lock(&lcd_mutex, K_FOREVER);
        snprintf(display_buffer, sizeof(display_buffer), "Status: %s", get_system_status());
        lcd_print(display_buffer);
        k_mutex_unlock(&lcd_mutex);
        
        k_sleep(K_SECONDS(3.5));
    }
}

K_THREAD_DEFINE(temp_display_tid, LCD_STACK_SIZE, temperature_display_thread,
                NULL, NULL, NULL, LCD_PRIORITY, 0, 0);

K_THREAD_DEFINE(hum_display_tid, LCD_STACK_SIZE, humidity_display_thread,
                NULL, NULL, NULL, LCD_PRIORITY, 0, 0);

K_THREAD_DEFINE(status_display_tid, LCD_STACK_SIZE, status_display_thread,
                NULL, NULL, NULL, LCD_PRIORITY, 0, 0);

void mutex_lcd_init(void) {
    printk("MUTEX ZADATAK: Pokrenut\n");
}
