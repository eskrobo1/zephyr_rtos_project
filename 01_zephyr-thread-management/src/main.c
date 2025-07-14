#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

/* Deklaracije za statičke niti */
void stop_static_threads(void);

/* Deklaracije za dinamičke niti */
void create_dynamic_threads(void);
void stop_dynamic_threads(void);
void scheduling_tests_init(void);

int main(void)
{
    printk("\n================================================\n");
    printk("=== Zephyr Task Management Demo ===\n");
    printk("================================================\n");
    printk("Ploca: %s\n", CONFIG_BOARD);
    printk("Frekvencija sistema: %d Hz\n", CONFIG_SYS_CLOCK_TICKS_PER_SEC);
    
    /* Demonstracija osnovnih taskova */
    printk("\n--- Testiranje statičkih i dinamičkih niti ---\n");
    printk("\n--- STATIČKE NITI ---\n");
    /* Statičke niti su već pokrenute automatski putem K_THREAD_DEFINE */
    k_sleep(K_SECONDS(15));
    stop_static_threads();
    
    printk("\n--- DINAMIČKE NITI ---\n");
    create_dynamic_threads();
    k_sleep(K_SECONDS(15));
    stop_dynamic_threads();
    
    /* Demonstracija scheduling politika */
    printk("\n--- Demonstracija scheduling politika ---\n");
    scheduling_tests_init();
    
    printk("\n=== Gotovi svi testovi ===\n");
    
    /* Nastavi izvršavanje osnovnih taskova */
    while (1) {
        k_sleep(K_SECONDS(10));
    }
    
    return 0;
}

/* Pomoćna funkcija za ispis vremena */
void print_timing_info(const char *task_name, uint32_t expected_ms, uint32_t actual_ms)
{
    int32_t deviation = actual_ms - expected_ms;
    int percent = (deviation * 100) / expected_ms;
    
    if (abs(percent) < 5) {
        printk("[TAJMING] %s: Ocekivano %u ms, Stvarno %u ms - OK (odstupanje: %d%%)\n",
               task_name, expected_ms, actual_ms, percent);
    } else {
        printk("[TAJMING] %s: Ocekivano %u ms, Stvarno %u ms - UPOZORENJE (odstupanje: %d%%)\n",
               task_name, expected_ms, actual_ms, percent);
    }
}
