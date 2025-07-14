#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>

/* GPIO definicije za Raspberry Pi 4B */
#define LED0_NODE DT_ALIAS(led0)
#if !DT_NODE_HAS_STATUS(LED0_NODE, okay)
#error "Unsupported board: led0 devicetree alias is not defined"
#endif

static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);

/* Globalne varijable */
static uint32_t led_toggle_count = 0;
static bool led_state = false;

/* Timer 1 - LED Controller (1 sekunda) */
void led_timer_handler(struct k_timer *timer_id)
{
    int ret;
    
    led_state = !led_state;
    led_toggle_count++;
    
    ret = gpio_pin_set_dt(&led, led_state);
    if (ret < 0) {
        printk("Greška pri postavljanju LED pina\n");
    }
}

K_TIMER_DEFINE(led_timer, led_timer_handler, NULL);

/* Timer 2 - Status Reporter (5 sekundi) */
void status_timer_handler(struct k_timer *timer_id)
{
    uint32_t uptime = k_uptime_get_32() / 1000;
    
    printk("\n=== STATUS REPORT ===\n");
    printk("System uptime: %u sekundi\n", uptime);
    printk("LED toggle count: %u\n", led_toggle_count);
    printk("LED trenutno: %s\n", led_state ? "ON" : "OFF");
    printk("===================\n\n");
}

K_TIMER_DEFINE(status_timer, status_timer_handler, NULL);

int main(void)
{
    int ret;
    
    printk("\n=== ZEPHYR SOFTVERSKI TIMERI DEMO ===\n");
    printk("Platform: Raspberry Pi 4B\n\n");
    
    /* Inicijalizacija GPIO za LED */
    if (!gpio_is_ready_dt(&led)) {
        printk("LED GPIO nije spreman!\n");
        return -1;
    }
    
    ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);
    if (ret < 0) {
        printk("Greška pri konfiguraciji LED pina: %d\n", ret);
        return -1;
    }
    
    /* Početno stanje LED - ugašen */
    gpio_pin_set_dt(&led, 0);
    
    printk("GPIO inicijalizovan - LED na pinu GPIO%d\n", led.pin);
    
    /* Pokretanje timera */
    k_timer_start(&led_timer, K_SECONDS(1), K_SECONDS(1));
    printk("LED timer pokrenut - period 1s\n");
    
    k_timer_start(&status_timer, K_SECONDS(5), K_SECONDS(5));
    printk("Status timer pokrenut - period 5s\n");
    
    printk("\nSistem je spreman!\n\n");
    
    while (1) {
        k_sleep(K_FOREVER);
    }
    
    return 0;
}
