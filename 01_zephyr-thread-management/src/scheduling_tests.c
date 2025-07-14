#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

/* Stack za test taskove */
K_THREAD_STACK_DEFINE(test1_stack, 1024);
K_THREAD_STACK_DEFINE(test2_stack, 1024);
static struct k_thread test1_data;
static struct k_thread test2_data;

static volatile bool test_running = true;
static volatile int shared_counter = 0;

/* Test task koji ne prepušta kontrolu */
void blocking_task(void *p1, void *p2, void *p3)
{
    int task_id = POINTER_TO_INT(p1);
    ARG_UNUSED(p2);
    ARG_UNUSED(p3);
    
    printk("[TEST BLOKIRANJA] Task %d pokrenut\n", task_id);
    
    for (int i = 0; i < 3 && test_running; i++) {
        int64_t start = k_uptime_get();
        
        /* Busy wait 1s - simulacija blokiranja */
        while ((k_uptime_get() - start) < 1000) {
            shared_counter++;
        }
        
        printk("[%3lld s] Blokirajuci Task %d - iteracija %d, brojac=%d\n", 
               k_uptime_get() / 1000, task_id, i+1, shared_counter);
    }
    
    printk("[TEST BLOKIRANJA] Task %d zavrsen\n", task_id);
}

/* Test za preemptivno raspoređivanje */
void test_preemption(void)
{
    printk("\n=== TEST: PREEMPTIVNO RASPOREDIVANJE ===\n");
    printk("Kreiranje 2 taska sa istim POZITIVNIM prioritetom (preemptivno)...\n");
    printk("Konfiguracija: CONFIG_TIMESLICING=%s\n", 
           IS_ENABLED(CONFIG_TIMESLICING) ? "omogućeno" : "onemogućeno");
    
    shared_counter = 0;
    test_running = true;
    
    k_thread_create(&test1_data, test1_stack,
                    K_THREAD_STACK_SIZEOF(test1_stack),
                    blocking_task, INT_TO_POINTER(1), NULL, NULL,
                    7, 0, K_NO_WAIT);  // Pozitivni prioritet = preemptivno
    
    k_thread_create(&test2_data, test2_stack,
                    K_THREAD_STACK_SIZEOF(test2_stack),
                    blocking_task, INT_TO_POINTER(2), NULL, NULL,
                    7, 0, K_NO_WAIT);  // Pozitivni prioritet = preemptivno
    
    k_sleep(K_SECONDS(6));
    test_running = false;
    k_sleep(K_MSEC(100));
    
    printk("Test zavrsen. Finalni brojac: %d\n", shared_counter);
    printk("=========================================\n\n");
}

/* Test za cooperativno raspoređivanje */
void test_cooperative(void)
{
    printk("\n=== TEST: COOPERATIVNO RASPOREDIVANJE ===\n");
    printk("Kreiranje 2 taska sa istim NEGATIVNIM prioritetom (cooperativno)...\n");
    
    shared_counter = 0;
    test_running = true;
    
    k_thread_create(&test1_data, test1_stack,
                    K_THREAD_STACK_SIZEOF(test1_stack),
                    blocking_task, INT_TO_POINTER(1), NULL, NULL,
                    -7, 0, K_NO_WAIT);  // Negativni prioritet = cooperativno
    
    k_thread_create(&test2_data, test2_stack,
                    K_THREAD_STACK_SIZEOF(test2_stack),
                    blocking_task, INT_TO_POINTER(2), NULL, NULL,
                    -7, 0, K_NO_WAIT);  // Negativni prioritet = cooperativno
    
    k_sleep(K_SECONDS(6));
    test_running = false;
    k_sleep(K_MSEC(100));
    
    printk("Test zavrsen. Finalni brojac: %d\n", shared_counter);
    printk("=========================================\n\n");
}

/* Test za prioritete */
void priority_task(void *p1, void *p2, void *p3)
{
    int task_id = POINTER_TO_INT(p1);
    int priority = POINTER_TO_INT(p2);
    ARG_UNUSED(p3);
    
    for (int i = 0; i < 3; i++) {
        printk("[%3lld s] Prioritetni Task %d (prio=%d) - izvrsavanje %d\n", 
               k_uptime_get() / 1000, task_id, priority, i+1);
        k_sleep(K_SECONDS(1));
    }
}

void test_priorities_preemptive(void)
{
    printk("\n=== TEST: PREEMPTIVNI PRIORITETI ===\n");
    printk("Task 1: prioritet 5 (viši), Task 2: prioritet 10 (niži)\n");
    
    k_tid_t tid1 = k_thread_create(&test1_data, test1_stack,
                    K_THREAD_STACK_SIZEOF(test1_stack),
                    priority_task, INT_TO_POINTER(1), INT_TO_POINTER(5), NULL,
                    5, 0, K_FOREVER);  // Pozitivni = preemptivni
    
    k_tid_t tid2 = k_thread_create(&test2_data, test2_stack,
                    K_THREAD_STACK_SIZEOF(test2_stack),
                    priority_task, INT_TO_POINTER(2), INT_TO_POINTER(10), NULL,
                    10, 0, K_FOREVER);  // Pozitivni = preemptivni
    
    k_thread_start(tid1);
    k_thread_start(tid2);
    
    k_sleep(K_SECONDS(4));
    
    printk("==========================================\n\n");
}

void test_priorities_cooperative(void)
{
    printk("\n=== TEST: COOPERATIVNI PRIORITETI ===\n");
    printk("Task 1: prioritet -5 (niži), Task 2: prioritet -10 (viši)\n");
    
    k_tid_t tid1 = k_thread_create(&test1_data, test1_stack,
                    K_THREAD_STACK_SIZEOF(test1_stack),
                    priority_task, INT_TO_POINTER(1), INT_TO_POINTER(-5), NULL,
                    -5, 0, K_FOREVER);  // Negativni = cooperativni
    
    k_tid_t tid2 = k_thread_create(&test2_data, test2_stack,
                    K_THREAD_STACK_SIZEOF(test2_stack),
                    priority_task, INT_TO_POINTER(2), INT_TO_POINTER(-10), NULL,
                    -10, 0, K_FOREVER);  // Negativni = cooperativni
    
    k_thread_start(tid1);
    k_thread_start(tid2);
    
    k_sleep(K_SECONDS(4));
    
    printk("==========================================\n\n");
}

void scheduling_tests_init(void)
{
    printk("\n=== ZEPHYR SCHEDULING TESTS ===\n");
    printk("CONFIG_TIMESLICING: %s\n", 
           IS_ENABLED(CONFIG_TIMESLICING) ? "DA" : "NE");
    printk("===============================\n");
    
    test_preemption();        // Pozitivni prioriteti + busy wait
    test_cooperative();       // Negativni prioriteti + busy wait  
    test_priorities_preemptive();   // Različiti pozitivni prioriteti
    test_priorities_cooperative();  // Različiti negativni prioriteti
}
