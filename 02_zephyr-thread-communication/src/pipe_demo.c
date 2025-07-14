#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <string.h>

/* Pipe definicija - buffer od 256 bajtova */
K_PIPE_DEFINE(counter_pipe, 256, 4);

/* Stack definicije za pipe demo taskove */
K_THREAD_STACK_DEFINE(pipe_task1_stack, 1024);
K_THREAD_STACK_DEFINE(pipe_task2_stack, 1024);

/* Thread strukture */
static struct k_thread pipe_task1_thread;
static struct k_thread pipe_task2_thread;

/* Kontrolna promenljiva za zaustavljanje threadova */
static volatile bool pipe_demo_running = false;

/* Struktura za prenos podataka */
struct pipe_message {
    uint32_t counter;
    uint32_t timestamp;
} __packed;

/* Statistike za pipe demo */
static struct {
    uint32_t messages_sent;
    uint32_t messages_received;
    uint32_t partial_transfers;
} pipe_stats = {0};

/* Helper funkcija za formatiranje vremena */
static void print_time(void)
{
    uint32_t ms = k_uptime_get_32();
    printk("[%3u.%03u s] ", ms / 1000, ms % 1000);
}

/* Task 1: Producer - šalje strukturirane podatke svake 3 sekunde */
static void pipe_producer_task(void *p1, void *p2, void *p3)
{
    struct pipe_message msg;
    size_t bytes_written;
    uint32_t counter = 0;
    int ret;
    
    print_time();
    printk("Pipe Task1 pokrenut\n");
    
    while (pipe_demo_running) {
        counter++;
        
        /* Pripremi poruku */
        msg.counter = counter;
        msg.timestamp = k_uptime_get_32();
        
        /* Pošalji kroz pipe */
        ret = k_pipe_put(&counter_pipe, &msg, sizeof(msg), 
                        &bytes_written, sizeof(msg), K_MSEC(100));
        
        if (ret == 0 && bytes_written == sizeof(msg)) {
            pipe_stats.messages_sent++;
            print_time();
            printk("Task1 šalje: %u\n", counter);
        } else if (ret == 0 && bytes_written < sizeof(msg)) {
            pipe_stats.partial_transfers++;
            print_time();
            printk("Task1 PARTIAL: %u (samo %zu/%zu bajtova)\n", 
                   counter, bytes_written, sizeof(msg));
        } else {
            print_time();
            printk("Task1 FAILED: %u\n", counter);
        }
        
        /* Proveri da li treba stati pre čekanja */
        for (int i = 0; i < 30 && pipe_demo_running; i++) {
            k_msleep(100);
        }
    }
}

/* Task 2: Consumer - čita strukturirane podatke svake 7 sekunde */
static void pipe_consumer_task(void *p1, void *p2, void *p3)
{
    struct pipe_message msg;
    size_t bytes_read;
    int ret;
    int count;
    
    print_time();
    printk("Pipe Task2 pokrenut\n");
    
    while (pipe_demo_running) {
        /* Čekanje u manjim intervalima za brži prekid */
        for (int i = 0; i < 70 && pipe_demo_running; i++) {
            k_msleep(100);
        }
        
        if (!pipe_demo_running) break;
        
        count = 0;
        print_time();
        printk("Task2 čita: ");
        
        /* Čitaj sve dostupne kompletne poruke */
        while (1) {
            ret = k_pipe_get(&counter_pipe, &msg, sizeof(msg),
                            &bytes_read, sizeof(msg), K_NO_WAIT);
            
            if (ret != 0 || bytes_read < sizeof(msg)) {
                break;
            }
            
            if (count > 0) printk(", ");
            printk("%u", msg.counter);
            count++;
            pipe_stats.messages_received++;
        }
        
        if (count == 0) {
            printk("(prazno)");
        }
        printk(" [ukupno: %d]\n", count);
    }
}

/* Javna funkcija za inicijalizaciju pipe demo */
void pipe_demo_init(void)
{
    printk("\n--- PIPE DEMO ---\n");
    
    /* Resetuj statistike */
    memset(&pipe_stats, 0, sizeof(pipe_stats));
    
    /* Isprazni pipe */
    k_pipe_flush(&counter_pipe);
    
    pipe_demo_running = true;
    
    /* Kreiraj producer task */
    k_thread_create(&pipe_task1_thread, pipe_task1_stack,
                    K_THREAD_STACK_SIZEOF(pipe_task1_stack),
                    pipe_producer_task, NULL, NULL, NULL,
                    5, 0, K_NO_WAIT);
    k_thread_name_set(&pipe_task1_thread, "pipe_producer");
    
    /* Kreiraj consumer task */
    k_thread_create(&pipe_task2_thread, pipe_task2_stack,
                    K_THREAD_STACK_SIZEOF(pipe_task2_stack),
                    pipe_consumer_task, NULL, NULL, NULL,
                    5, 0, K_NO_WAIT);
    k_thread_name_set(&pipe_task2_thread, "pipe_consumer");
    
    /* Čekaj da se demo završi (poziva se iz main-a) */
    while (pipe_demo_running) {
        k_msleep(1000);
    }
    
    /* Sačekaj da se threadovi završe */
    k_thread_join(&pipe_task1_thread, K_SECONDS(2));
    k_thread_join(&pipe_task2_thread, K_SECONDS(2));
    
    /* Prikaži finalne statistike */
    print_time();
    printk("Pipe rezultat: poslano=%u, primljeno=%u, parcijalno=%u\n",
           pipe_stats.messages_sent,
           pipe_stats.messages_received,
           pipe_stats.partial_transfers);
}

/* Javna funkcija za zaustavljanje demo-a */
void pipe_demo_stop(void)
{
    pipe_demo_running = false;
}
