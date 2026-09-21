#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include "app_queues.h"
#include "parser.h"
#include "system_zero.h"
#include "system_one.h"

#define STACK_SIZE 1024
#define THREAD_PRIORITY 7

K_MSGQ_DEFINE(rgb_msgq, sizeof(struct rgb_cmd), 10, 4);
K_MSGQ_DEFINE(servo_msgq, sizeof(struct servo_cmd), 10, 4);

K_THREAD_DEFINE(parser_tid, STACK_SIZE, parser_thread_entry, NULL, NULL, NULL, THREAD_PRIORITY, 0, 0);
K_THREAD_DEFINE(sys0_tid, STACK_SIZE, system_zero_entry, NULL, NULL, NULL, THREAD_PRIORITY, 0, 0);
K_THREAD_DEFINE(sys1_tid, STACK_SIZE, system_one_entry, NULL, NULL, NULL, THREAD_PRIORITY, 0, 0);
int main(void){
    printk("\n========================\n");
    printk("System startup complete.\n"); 
    printk("========================\n");
    return 0;
}