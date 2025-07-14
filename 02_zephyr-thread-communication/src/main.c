#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

extern void queue_demo_init(void);
extern void pipe_demo_init(void);
extern void queue_demo_stop(void);
extern void pipe_demo_stop(void);

/* Dužina trajanja svake demonstracije */
#define DEMO_DURATION_SEC    30  /* 30 sekundi po demonstraciji */

/* Thread za pokretanje demo-a */
K_THREAD_STACK_DEFINE(demo_stack, 2048);
struct k_thread demo_thread;

/* Funkcija koja pokreće demo u zasebnom thread-u */
void demo_runner(void *init_func, void *stop_func, void *unused)
{
    void (*init)(void) = init_func;
    init();
}

void run_demo_with_timer(const char *demo_name, void (*init_func)(void), void (*stop_func)(void))
{
    /* Kreiranje thread za demo */
    k_tid_t tid = k_thread_create(&demo_thread, demo_stack,
                                  K_THREAD_STACK_SIZEOF(demo_stack),
                                  demo_runner, init_func, stop_func, NULL,
                                  4, 0, K_NO_WAIT);
    
    /* Čekanje dok demo traje */
    k_msleep(DEMO_DURATION_SEC * 1000);
    
    /* Zaustavljanje*/
    stop_func();
    k_thread_join(&demo_thread, K_SECONDS(5));
    
    /* Pauza između demonstracija */
    k_msleep(3000);
}

void main(void)
{
    printk("\n=== Zephyr IPC Demo ===\n");
    printk("Task1: 3s period | Task2: 7s period\n");
    printk("Trajanje: %d sekundi po demo\n", DEMO_DURATION_SEC);
    
    /* Pauza za stabilizaciju sistema */
    k_msleep(1000);
    
    /* Pokretanje Message Queue Demo */
    run_demo_with_timer("Message Queue", queue_demo_init, queue_demo_stop);
    
    /* Pokretanje Pipe Demo */
    run_demo_with_timer("Pipe", pipe_demo_init, pipe_demo_stop);
    
    printk("\n=== DEMO ZAVRŠEN ===\n");
    
    /* Main thread ostaje živ ali idle */
    while (1) {
        k_msleep(60000);
    }
}
