#include "z_tool.h"
#include "z_debug.h"
#include "z_thpool.h"

#define MAX_POOLS 10

// Help text for command-line interface
static const char *gs_help =
    "\r\n"
    "Enter a command (type 'exit' to quit):\r\n\r\n"
    "create pool1 5 100 64  #Create pool named 'pool1' with 5 threads, 100 cache queues, 64k stack size\r\n"
    "destroy pool1          #Destroy pool named 'pool1'\r\n"
    "add pool1 10           #Add 10 tasks to pool named 'pool1'\r\n"
    "show pool1             #Show state of pool named 'pool1'\r\n"
    "test                   #Run test suite\r\n"
    "help                   #Show this help\r\n";

// Structure to track thread pools
struct pool_entry {
    char name[32];
    z_thpool_handle_t handle;
    int in_use;
};

// Array of thread pools
static struct pool_entry gs_pools[MAX_POOLS] = {0};

// Simulate a task by sleeping for 1 second
static void test_task_cb(void *p_arg) {
    sleep(1); // Simulating task processing time
}

// Find a pool by name
static struct pool_entry *find_pool(const char *name) {
    for (int i = 0; i < MAX_POOLS; i++) {
        if (gs_pools[i].in_use && strcmp(gs_pools[i].name, name) == 0) {
            return &gs_pools[i];
        }
    }
    return NULL;
}

// Find an empty pool slot
static struct pool_entry *find_empty_slot(void) {
    for (int i = 0; i < MAX_POOLS; i++) {
        if (!gs_pools[i].in_use) {
            return &gs_pools[i];
        }
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    char input[256];
    char command[32];
    char pool_name[32];
    int a, b, c;

    printf("Enter a command (type 'exit' to quit):\n");

    while (1) {
        printf("> ");
        if (fgets(input, sizeof(input), stdin) == NULL) {
            printf("Error reading input. Exiting.\n");
            break;
        }

        input[strcspn(input, "\n")] = '\0';

        if (strcmp(input, "exit") == 0) {
            printf("Exiting program.\n");
            break;
        } else if (strcmp(input, "help") == 0) {
            printf("%s", gs_help);
            continue;
        } else if (strcmp(input, "test") == 0) {
            z_thpool_test();
            continue;
        }

        // Parse commands with pool name
        if (sscanf(input, "%s %s %d %d %d", command, pool_name, &a, &b, &c) >= 2) {
            if (strcmp(command, "create") == 0) {
                struct pool_entry *entry = find_empty_slot();
                if (!entry) {
                    printf("Error: Maximum number of pools reached\n");
                    continue;
                }

                struct z_thpool_config_struct config;
                config.max_thread_nums = a;
                config.msg_node_max = b;
                config.thread_stack_size = c * 1024;
                strncpy(config.pool_name, pool_name, sizeof(config.pool_name) - 1);
                config.pool_name[sizeof(config.pool_name) - 1] = '\0';

                z_thpool_handle_t handle;
                if (z_thpool_create(&config, &handle) == 0) {
                    strncpy(entry->name, pool_name, sizeof(entry->name) - 1);
                    entry->name[sizeof(entry->name) - 1] = '\0';
                    entry->handle = handle;
                    entry->in_use = 1;
                    printf("Created thread pool '%s'\n", pool_name);
                } else {
                    printf("Failed to create thread pool\n");
                }
            } else {
                struct pool_entry *entry = find_pool(pool_name);
                if (!entry) {
                    printf("Error: Pool '%s' not found\n", pool_name);
                    continue;
                }

                if (strcmp(command, "destroy") == 0) {
                    z_thpool_destroy(entry->handle);
                    entry->in_use = 0;
                    printf("Destroyed thread pool '%s'\n", pool_name);
                } else if (strcmp(command, "add") == 0) {
                    for (int i = 0; i < a; i++) {
                        if (z_thpool_add_work(entry->handle, test_task_cb, &i) != 0) {
                            printf("Failed to add task %d\n", i);
                            break;
                        }
                        printf("Added task %d to pool '%s'\n", i, pool_name);
                    }
                } else if (strcmp(command, "show") == 0) {
                    z_thpool_cmd_shell_show(entry->handle);
                } else {
                    printf("%s", gs_help);
                }
            }
        } else {
            printf("%s", gs_help);
        }
    }

    // Cleanup any remaining pools
    for (int i = 0; i < MAX_POOLS; i++) {
        if (gs_pools[i].in_use) {
            z_thpool_destroy(gs_pools[i].handle);
            gs_pools[i].in_use = 0;
        }
    }

    return 0;
}