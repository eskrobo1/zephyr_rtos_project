#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>
#include <zephyr/devicetree.h>

#define LED_PIN 17

// Definiranje node ID za gpio0 pomoću labele
#define GPIO_NODE DT_NODELABEL(gpio0)

// Provjera da li node postoji
#if !DT_NODE_HAS_STATUS(GPIO_NODE, okay)
#error "GPIO node 'gpio0' is not enabled in the device tree"
#endif


static uint32_t led_toggle_count = 0;
static bool led_state = false;

// device pointer
const struct device *gpio_dev;

/* Timer 1 - LED Controller (1 sekunda) */
void led_timer_handler(struct k_timer *timer_id)
{
    uint32_t uptime = k_uptime_get_32();
    
    led_state = !led_state;
    led_toggle_count++;
    
    if (gpio_dev) {
        gpio_pin_set(gpio_dev, LED_PIN, led_state);
    }
    
    printk("[%6d.%03d] LED: %s (promjena #%u)\n", 
           uptime/1000, uptime%1000,
           led_state ? "ON" : "OFF", 
           led_toggle_count);
}

K_TIMER_DEFINE(led_timer, led_timer_handler, NULL);

/* Timer 2 - Status Reporter (5 sekundi) */
void status_timer_handler(struct k_timer *timer_id)
{
    uint32_t uptime = k_uptime_get_32() / 1000;
    
    printk("\n[%6d.000] === STATUS REPORT ===\n", uptime);
    printk("            Vrijeme rada sistema: %u sekundi\n", uptime);
    printk("            Broj LED promjena: %u\n", led_toggle_count);
    printk("            LED trenutno: %s\n", led_state ? "ON" : "OFF");
    printk("            ====================\n\n");
}

K_TIMER_DEFINE(status_timer, status_timer_handler, NULL);

int main(void)
{
    int ret;
    gpio_dev = DEVICE_DT_GET(GPIO_NODE);

    if (!gpio_dev) {
        printk("Greška: Nije moguće dohvatiti GPIO device 'gpio0'.\n");
        printk("Nastavljam bez hardverske LED kontrole\n");
        return 1;
    } else {
        ret = gpio_pin_configure(gpio_dev, LED_PIN, GPIO_OUTPUT_ACTIVE);
        if (ret < 0) {
            printk("Greška pri konfiguraciji GPIO%d: %d\n", LED_PIN, ret);
            gpio_dev = NULL;
        } else {
            gpio_pin_set(gpio_dev, LED_PIN, 0);
            printk("GPIO inicijalizovan - LED na pinu GPIO%d\n", LED_PIN);
        }
    }
    
    printk("Timer 1: LED kontrola - period 1s\n");
    printk("Timer 2: Status report - period 5s\n");
    
    k_timer_start(&led_timer, K_SECONDS(1), K_SECONDS(1));
    printk("LED timer pokrenut\n");
    
    k_timer_start(&status_timer, K_SECONDS(5), K_SECONDS(5));
    printk("Status timer pokrenut\n");
    
    while (1) {
        k_sleep(K_FOREVER);
    }
    
    return 0;
}
