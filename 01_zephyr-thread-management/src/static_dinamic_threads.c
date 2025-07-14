#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

/* Eksterna funkcija za ispis timing informacija */
extern void print_timing_info(const char *thread_name, uint32_t expected_ms, uint32_t actual_ms);


#define STATIC_STACK_SIZE 1024
#define STATIC_PRIORITY 5

static volatile bool static_threads_running = true;
static uint32_t static_thread1_count = 0;
static uint32_t static_thread2_count = 0;
static int64_t static_thread1_last_execution = 0;
static int64_t static_thread2_last_execution = 0;

/* Statička nit 1 - Period 3 sekunde */
void static_thread1_entry(void)
{   
    printk("[STATIČKA NIT 1] Početak izvršavanja - period 3s\n");
    
    while (static_threads_running) {
        int64_t start_time = k_uptime_get();
        
        /* Prikaz vremena u sekundama */
        printk("[%3lld s] Statička Nit 1 - izvršavanje #%u\n", 
               start_time / 1000, ++static_thread1_count);
        
        /* Provjera perioda svako 5. izvršavanje */
        if (static_thread1_last_execution != 0 && static_thread1_count % 5 == 0) {
            uint32_t actual_period = start_time - static_thread1_last_execution;
            print_timing_info("Statička Nit 1", 3000, actual_period); 
        }
        static_thread1_last_execution = start_time;
        
        k_sleep(K_SECONDS(3));
    }
    printk("[STATIČKA NIT 1] Završava izvršavanje\n");
}

/* Statička nit 2 - Period 7 sekundi */
void static_thread2_entry(void)
{
    printk("[STATIČKA NIT 2] Početak izvršavanja - period 7s\n");
    
    while (static_threads_running) {
        int64_t start_time = k_uptime_get();
        
        printk("[%3lld s] Statička Nit 2 - izvršavanje #%u\n", 
               start_time / 1000, ++static_thread2_count);
        
        /* Provjera perioda svako 3. izvršavanje */
        if (static_thread2_last_execution != 0 && static_thread2_count % 3 == 0) {
            uint32_t actual_period = start_time - static_thread2_last_execution;
            print_timing_info("Statička Nit 2", 7000, actual_period); 
        }
        static_thread2_last_execution = start_time;
        
        k_sleep(K_SECONDS(7));
    }
    printk("[STATIČKA NIT 2] Završava izvršavanje\n");
}

/* DEFINISANJE STATIČKIH NITI - sve se kreira automatski */
K_THREAD_DEFINE(static_thread1_id, STATIC_STACK_SIZE,
                static_thread1_entry, NULL, NULL, NULL,
                STATIC_PRIORITY, 0, 0);

K_THREAD_DEFINE(static_thread2_id, STATIC_STACK_SIZE,
                static_thread2_entry, NULL, NULL, NULL,
                STATIC_PRIORITY, 0, 0);

/* Funkcija za zaustavljanje statičkih niti */
void stop_static_threads(void)
{
    static_threads_running = false;
    
    // Čekamo da se niti završe
    k_thread_join(static_thread1_id, K_SECONDS(5));
    k_thread_join(static_thread2_id, K_SECONDS(5));
    
    printk("Statičke niti su zaustavljene\n");
}


#define DYNAMIC_STACK_SIZE 1024
#define DYNAMIC_PRIORITY 6

/* Stack definicije za dinamičke niti */
K_THREAD_STACK_DEFINE(dynamic_thread1_stack, DYNAMIC_STACK_SIZE);
K_THREAD_STACK_DEFINE(dynamic_thread2_stack, DYNAMIC_STACK_SIZE);

/* Thread kontrolni blokovi */
static struct k_thread dynamic_thread1_data;
static struct k_thread dynamic_thread2_data;

static k_tid_t dynamic_thread1_tid;
static k_tid_t dynamic_thread2_tid;

static volatile bool dynamic_threads_running = false;
static uint32_t dynamic_thread1_count = 0;
static uint32_t dynamic_thread2_count = 0;
static int64_t dynamic_thread1_last_execution = 0;
static int64_t dynamic_thread2_last_execution = 0;

/* Dinamička nit 1 - Period 2 sekunde */
void dynamic_thread1_entry(void)
{   
    printk("[DINAMIČKA NIT 1] Početak izvršavanja - period 3s\n");
    
    while (dynamic_threads_running) {
        int64_t start_time = k_uptime_get();
        
        /* Prikaz vremena u sekundama */
        printk("[%3lld s] Dinamička Nit 1 - izvršavanje #%u\n", 
               start_time / 1000, ++dynamic_thread1_count);
        
        /* Provjera perioda svako 5. izvršavanje */
        if (dynamic_thread1_last_execution != 0 && dynamic_thread1_count % 5 == 0) {
            uint32_t actual_period = start_time - dynamic_thread1_last_execution;
            print_timing_info("Dinamička Nit 1", 3000, actual_period); 
        }
        dynamic_thread1_last_execution = start_time;
        
        k_sleep(K_SECONDS(3));
    }
    printk("[DINAMIČKA NIT 1] Završava izvršavanje\n");
}

/* Dinamička nit 2 - Period 5 sekundi */
void dynamic_thread2_entry(void)
{
    printk("[DINAMIČKA NIT 2] Početak izvršavanja - period 7s\n");
    
    while (dynamic_threads_running) {
        int64_t start_time = k_uptime_get();
        
        printk("[%3lld s] Dinamička Nit 2 - izvršavanje #%u\n", 
               start_time / 1000, ++dynamic_thread2_count);
        
        /* Provjera perioda svako 3. izvršavanje */
        if (dynamic_thread2_last_execution != 0 && dynamic_thread2_count % 3 == 0) {
            uint32_t actual_period = start_time - dynamic_thread2_last_execution;
            print_timing_info("Dinamička Nit 2", 7000, actual_period); 
        }
        dynamic_thread2_last_execution = start_time;
        
        k_sleep(K_SECONDS(7));
    }
    printk("[DINAMIČKA NIT 2] Završava izvršavanje\n");
}

/* Kreiranje dinamičkih niti */
void create_dynamic_threads(void)
{
    
    dynamic_threads_running = true;
    
    /* Reset brojača prije kreiranja */
    dynamic_thread1_count = 0;
    dynamic_thread2_count = 0;
    dynamic_thread1_last_execution = 0;
    dynamic_thread2_last_execution = 0;
    
    dynamic_thread1_tid = k_thread_create(&dynamic_thread1_data, 
                                         dynamic_thread1_stack,
                                         K_THREAD_STACK_SIZEOF(dynamic_thread1_stack),
                                         dynamic_thread1_entry, 
                                         NULL, NULL, NULL,
                                         DYNAMIC_PRIORITY, 0, K_NO_WAIT);
    
    dynamic_thread2_tid = k_thread_create(&dynamic_thread2_data, 
                                         dynamic_thread2_stack,
                                         K_THREAD_STACK_SIZEOF(dynamic_thread2_stack),
                                         dynamic_thread2_entry, 
                                         NULL, NULL, NULL,
                                         DYNAMIC_PRIORITY, 0, K_NO_WAIT);
}

void stop_dynamic_threads(void)
{
    dynamic_threads_running = false;
    
    k_thread_join(dynamic_thread1_tid, K_SECONDS(5));
    k_thread_join(dynamic_thread2_tid, K_SECONDS(5));
    
    printk("Dinamičke niti su zaustavljene\n");
}
