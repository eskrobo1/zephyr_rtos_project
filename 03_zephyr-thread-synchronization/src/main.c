#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include "shared_data.h"

/* Deklaracije funkcija iz drugih fajlova */
extern void mutex_lcd_init(void);
extern void sem_sensor_init(void);

int main(void) {
    printk("\n\n========================================\n");
    printk("    ZEPHYR MUTEX I SEMAFOR DEMO\n");
    printk("========================================\n");
    printk("MUTEX: 3 niti dijele LCD (2s, 3s, 4s)\n");
    printk("SEMAFOR: ISR->Worker komunikacija (800ms)\n");
    printk("LCD prikazuje STVARNE senzor vrijednosti!\n");
    printk("========================================\n\n");
    
    sensor_data_init();
    mutex_lcd_init();
    sem_sensor_init();
    
    printk("Svi taskovi pokrenuti!\n");
    printk("========================================\n\n");
    
    return 0;
}
