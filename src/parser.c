#include <zephyr/kernel.h>
#include <zephyr/console/console.h>
#include <zephyr/sys/printk.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "app_queues.h"
#include "parser.h"

static void trim_trailing(char* str){
    size_t len = strlen(str);
    while (len > 0 && (str[len - 1] == '\r' || str[len - 1] == '\n' || isspace((unsigned char)str[len - 1]))) {
        str[--len] = '\0';
    }
}

void parser_thread_entry(void *p1, void *p2, void *p3){
    ARG_UNUSED(p1);
    ARG_UNUSED(p2);
    ARG_UNUSED(p3);

    console_getline_init();    // init reader
    printk("[Parser] Parser thread started.\n");

    while(1){
        char *line = console_getline();
        if(line == NULL) continue; 

        trim_trailing(line);

        if(line[0] == '#' || strlen(line) == 6){    // #XXYYZZ or XXYYZZ format
            char *hex_str = (line[0] == '#') ? line + 1 : line; // skip # if its there

            if(strlen(hex_str) != 6){
                printk("[Parser] Invalid RGB command length: %s\n", line);
                continue;
            }

            unsigned int r, g, b; 
            if(sscanf(hex_str, "%02x%02x%02x", &r, &g, &b) == 3){
                struct rgb_cmd cmd = {
                    .red   =  (uint8_t ) r, 
                    .green =  (uint8_t ) g,
                    .blue  =  (uint8_t ) b
                };

                if(k_msgq_put(&rgb_msgq, &cmd, K_NO_WAIT) != 0){
                    printk("[Parser] Warning: RGB Queue full, dropping command \n"); 
                }
                else{
                    printk("[Parser] RGB command queued: R=%02X G=%02X B=%02X\n", cmd.red, cmd.green, cmd.blue);
                }
            }
            else{
                printk("[Parser] Invalid RGB command format: %s\n", line);
            }
        }
        else{
            printk("[Parser] Unrecognized command: %s\n", line);
        }
    }
}