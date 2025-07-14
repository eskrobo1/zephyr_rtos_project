#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

/* Message Queue definicija - kapacitet 10 poruka */
K_MSGQ_DEFINE(counter_msgq, sizeof(uint32_t), 10, 4);

/* Stack definicije za queue demo taskove */
K_THREAD_STACK_DEFINE(queue_task1_stack, 1024);
K_THREAD_STACK_DEFINE(queue_task2_stack, 1024);

/* Thread strukture */
static struct k_thread queue_task1_thread;
static struct k_thread queue_task2_thread;

/* Kontrolna promenljiva za zaustavljanje threadova */
static volatile bool queue_demo_running = false;

/* Statistike za queue demo */
static struct {
    uint32_t messages_sent;
    uint32_t messages_received;
    uint32_t messages_dropped;
    uint32_t max_queue_usage;
} queue_stats = {0};

/* Helper funkcija za formatiranje vremena */
static void print_time(void)
{
    uint32_t ms = k_uptime_get_32();
    printk("[%3u.%03u s] ", ms / 1000, ms % 1000);
}

/* Task 1: Producer - šalje brojač svake 3 sekunde */
static void queue_producer_task(void *p1, void *p2, void *p3)
{
    uint32_t counter = 0;
    int ret;
    
    print_time();
    printk("MsgQueue Task1 pokrenut\n");
    
    while (queue_demo_running) {
        counter++;
        
        /* Pokušaj slanja */
        ret = k_msgq_put(&counter_msgq, &counter, K_NO_WAIT);
        
        if (ret == 0) {
            queue_stats.messages_sent++;
            print_time();
            printk("Task1 šalje: %u\n", counter);
            
            /* Ažuriraj max queue usage */
            uint32_t used = k_msgq_num_used_get(&counter_msgq);
            if (used > queue_stats.max_queue_usage) {
                queue_stats.max_queue_usage = used;
            }
        } else {
            queue_stats.messages_dropped++;
            print_time();
            printk("Task1 DROPPED: %u (queue full)\n", counter);
        }
        
        /* Proveri da li treba stati pre čekanja */
        for (int i = 0; i < 30 && queue_demo_running; i++) {
            k_msleep(100);  /* Sleep u manjim intervalima */
        }
    }
}

/* Task 2: Consumer - čita brojače svake 7 sekunde */
static void queue_consumer_task(void *p1, void *p2, void *p3)
{
    uint32_t received_counter;
    int ret;
    int count = 0;
    
    print_time();
    printk("MsgQueue Task2 pokrenut\n");
    
    while (queue_demo_running) {
        /* Čekanje u manjim intervalima za brži prekid */
        for (int i = 0; i < 70 && queue_demo_running; i++) {
            k_msleep(100);
        }
        
        if (!queue_demo_running) break;
        
        count = 0;
        print_time();
        printk("Task2 čita: ");
        
        /* Čitaj sve dostupne poruke */
        while (1) {
            ret = k_msgq_get(&counter_msgq, &received_counter, K_NO_WAIT);
            
            if (ret != 0) {
                break;  /* Nema više poruka */
            }
            
            if (count > 0) printk(", ");
            printk("%u", received_counter);
            count++;
            queue_stats.messages_received++;
        }
        
        if (count == 0) {
            printk("(prazno)");
        }
        printk(" [ukupno: %d]\n", count);
    }
}

/* Javna funkcija za inicijalizaciju queue demo */
void queue_demo_init(void)
{
    printk("\n--- MESSAGE QUEUE DEMO ---\n");
    
    /* Resetuj statistike */
    memset(&queue_stats, 0, sizeof(queue_stats));
    
    /* Isprazni queue */
    k_msgq_purge(&counter_msgq);
    
    /* Postavi flag da demo radi */
    queue_demo_running = true;
    
    /* Kreiraj producer task */
    k_thread_create(&queue_task1_thread, queue_task1_stack,
                    K_THREAD_STACK_SIZEOF(queue_task1_stack),
                    queue_producer_task, NULL, NULL, NULL,
                    5, 0, K_NO_WAIT);
    k_thread_name_set(&queue_task1_thread, "queue_producer");
    
    /* Kreiraj consumer task */
    k_thread_create(&queue_task2_thread, queue_task2_stack,
                    K_THREAD_STACK_SIZEOF(queue_task2_stack),
                    queue_consumer_task, NULL, NULL, NULL,
                    5, 0, K_NO_WAIT);
    k_thread_name_set(&queue_task2_thread, "queue_consumer");
    
    /* Čekaj da se demo završi (poziva se iz main-a) */
    while (queue_demo_running) {
        k_msleep(1000);
    }
    
    /* Sačekaj da se threadovi završe */
    k_thread_join(&queue_task1_thread, K_SECONDS(2));
    k_thread_join(&queue_task2_thread, K_SECONDS(2));
    
    /* Prikaži finalne statistike */
    print_time();
    printk("MsgQueue rezultat: poslano=%u, primljeno=%u, izgubljeno=%u\n",
           queue_stats.messages_sent,
           queue_stats.messages_received,
           queue_stats.messages_dropped);
}

/* Javna funkcija za zaustavljanje demo-a */
void queue_demo_stop(void)
{
    queue_demo_running = false;
}
