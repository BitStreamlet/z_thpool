#include "z_tool.h"
#include "z_kfifo.h"
#include "z_debug.h"
#include "z_thpool.h"
#include "z_table_print.h"

#include <pthread.h>

#define Z_THPOOL_VERION "0.0.2.0"

// Structure to hold thread pool message details
struct z_thpool_msg_struct {
    void (*cb)(void *); // Callback function
    void *p_arg;        // Argument to the callback function
};

// Structure for managing the thread pool
struct z_thpool_mng_struct {
    int32_t start_flag;                     // Flag indicating if the thread pool is started
    struct z_kfifo_struct t_info;           // Information management for message queue
    int32_t th_run_flag;                    // Flag controlling thread pool run state
    uint32_t th_run_nums;                   // Number of currently running threads
    uint32_t th_busy_nums;                  // Number of busy threads
    uint32_t max_nums;                      // Maximum number of threads allowed
    uint32_t msg_node_max;                  // Maximum number of message nodes
    pthread_mutex_t mutex;                  // Mutex for synchronization
    pthread_cond_t cond;                    // Condition variable for thread synchronization
    uint32_t pub_bytes;                     // Data published (for statistics)
    uint32_t sub_bytes;                     // Data consumed (for statistics)
    struct z_thpool_config_struct t_config; // Configuration for thread pool
    char pool_name[32];                     // Name of the thread pool
};

// Global variables for thread pool management and synchronization
static struct z_thpool_mng_struct gs_thpool_mng = {0};
static pthread_mutex_t gs_thpool_mutex = PTHREAD_MUTEX_INITIALIZER;
static int32_t z_thpool_cmd(void);

// Static function declarations
static int32_t z_thpool_create_thread(pthread_t *p_pth, void *(*func)(void *), void *p_arg, uint32_t stack_size);
static void z_thpool_msg_read(void *param);
static void *z_thpool_proc(void *param);

/**
@brief Create a new thread pool instance
@param p_config Configuration parameters for the thread pool
@param p_handle Pointer to store the created thread pool handle
@return Status of thread pool creation, success is 0
*/
int32_t z_thpool_create(struct z_thpool_config_struct *p_config, z_thpool_handle_t *p_handle) {
    Z_DEBUG_ENTER();
    int32_t ret = -1;

    if (!p_config || !p_handle) {
        goto error0;
    }

    struct z_thpool_mng_struct *p_mng = (struct z_thpool_mng_struct *)calloc(1, sizeof(struct z_thpool_mng_struct));
    if (!p_mng) {
        goto error0;
    }

    // Initialize mutex and condition variable
    if (pthread_mutex_init(&p_mng->mutex, NULL) != 0) {
        goto error1;
    }

    if (pthread_cond_init(&p_mng->cond, NULL) != 0) {
        goto error2;
    }

    // Initialize FIFO queue
    ret = z_kfifo_malloc(&p_mng->t_info, sizeof(struct z_thpool_msg_struct) * p_config->msg_node_max);
    if (ret != 0) {
        goto error3;
    }

    // Copy configuration
    memcpy(&p_mng->t_config, p_config, sizeof(struct z_thpool_config_struct));
    strncpy(p_mng->pool_name, p_config->pool_name, sizeof(p_mng->pool_name) - 1);
    p_mng->pool_name[sizeof(p_mng->pool_name) - 1] = '\0';

    p_mng->max_nums = p_config->max_thread_nums;
    p_mng->msg_node_max = p_config->msg_node_max;
    p_mng->th_run_flag = 1;

    // Create worker threads
    for (uint32_t i = 0; i < p_config->max_thread_nums; i++) {
        pthread_t tid;
        ret = z_thpool_create_thread(&tid, z_thpool_proc, p_mng, p_config->thread_stack_size);
        if (ret != 0) {
            p_mng->th_run_flag = 0;
            pthread_cond_broadcast(&p_mng->cond);
            while (p_mng->th_run_nums > 0) {
                usleep(10000);
            }
            goto error4;
        }
        p_mng->th_run_nums++;
    }

    p_mng->start_flag = 1;
    *p_handle = p_mng;
    ret = 0;
    Z_DEBUG_EXIT(0);
    return ret;

error4:
    z_kfifo_free(&p_mng->t_info);
error3:
    pthread_cond_destroy(&p_mng->cond);
error2:
    pthread_mutex_destroy(&p_mng->mutex);
error1:
    free(p_mng);
error0:
    Z_DEBUG_EXIT(ret);
    return ret;
}

/**
@brief Destroy a thread pool instance
@param handle Handle to the thread pool to destroy
@return Status of thread pool destruction, success is 0
*/
int32_t z_thpool_destroy(z_thpool_handle_t handle) {
    Z_DEBUG_ENTER();
    int32_t ret = -1;

    if (!handle) {
        goto error0;
    }

    struct z_thpool_mng_struct *p_mng = (struct z_thpool_mng_struct *)handle;
    
    pthread_mutex_lock(&p_mng->mutex);
    if (!p_mng->start_flag) {
        pthread_mutex_unlock(&p_mng->mutex);
        goto error0;
    }

    p_mng->th_run_flag = 0;
    pthread_cond_broadcast(&p_mng->cond);
    pthread_mutex_unlock(&p_mng->mutex);

    // Wait for all threads to exit
    while (p_mng->th_run_nums > 0) {
        usleep(10000);
    }

    // Cleanup resources
    z_kfifo_free(&p_mng->t_info);
    pthread_mutex_destroy(&p_mng->mutex);
    pthread_cond_destroy(&p_mng->cond);
    free(p_mng);

    ret = 0;
    Z_DEBUG_EXIT(0);
    return ret;

error0:
    Z_DEBUG_EXIT(ret);
    return ret;
}

/**
@brief Create a thread, set its attributes, and start it
@param p_pth Pointer to the thread ID
@param func Function to be executed by the thread
@param p_arg Argument passed to the function
@param stack_size Stack size for the thread
@return Status of thread creation, success is 0
*/
static int32_t z_thpool_create_thread(pthread_t *p_pth, void *(*func)(void *), void *p_arg, uint32_t stack_size) {
    int32_t ret;
    pthread_attr_t attr;

    pthread_attr_init(&attr);

    // Set the thread to be detached
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_attr_setstacksize(&attr, stack_size);

#ifndef CFG_PLATFORM_X86
    // Set explicit scheduling if not on X86 platform
    ret = pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
    if (ret) {
        fprintf(stderr, "pthread_attr_setinheritsched:%d\n", ret);
        goto error;
    }
#endif

    // Create the thread with specified attributes
    ret = pthread_create(p_pth, &attr, func, p_arg);
    if (ret) {
        fprintf(stderr, "pthread_create:%d\n", ret);
        goto error;
    }

error:
    // Clean up thread attributes
    pthread_attr_destroy(&attr);
    return ret;
}

/**
@brief Read and process messages from the message queue
@param param Pointer to the thread pool management structure
@return No return value
*/
static void z_thpool_msg_read(void *param) {
    struct z_thpool_mng_struct *mng = (struct z_thpool_mng_struct *)param;
    struct z_thpool_msg_struct msg;

    pthread_mutex_lock(&mng->mutex);
    while (mng->th_run_flag && z_kfifo_data_len(&mng->t_info) < sizeof(struct z_thpool_msg_struct)) {
        pthread_cond_wait(&mng->cond, &mng->mutex);
    }

    if (mng->th_run_flag == 0 || z_kfifo_data_len(&mng->t_info) < sizeof(struct z_thpool_msg_struct)) {
        pthread_mutex_unlock(&mng->mutex);
        return;
    }

    // Retrieve message from the queue
    uint32_t len = z_kfifo_out(&mng->t_info, &msg, sizeof(struct z_thpool_msg_struct));
    mng->th_busy_nums++;
    mng->sub_bytes += len;
    pthread_mutex_unlock(&mng->mutex);

    // Execute the callback function for the message
    msg.cb(msg.p_arg);

    pthread_mutex_lock(&mng->mutex);
    mng->th_busy_nums--;
    pthread_mutex_unlock(&mng->mutex);
}

/**
@brief Thread pool processing function
@param param Pointer to the thread pool management structure
@return No return value
*/
static void *z_thpool_proc(void *param) {
    prctl(PR_SET_NAME, "thp");
    struct z_thpool_mng_struct *mng = (struct z_thpool_mng_struct *)param;

    // Continue processing messages as long as the run flag is set
    while (mng->th_run_flag) {
        z_thpool_msg_read(mng);
    }

    // Decrement the run number after processing is complete
    pthread_mutex_lock(&mng->mutex);
    mng->th_run_nums--;
    pthread_mutex_unlock(&mng->mutex);
    return NULL;
}

/**
@brief Add a work task to the thread pool
@param handle Handle to the thread pool
@param cb Callback function
@param p_arg Argument for the callback function
@return Status, success is 0
*/
int32_t z_thpool_add_work(z_thpool_handle_t handle, void (*cb)(void *), void *p_arg) {
    if (!handle || !cb || !p_arg) {
        return -EINVAL;
    }

    struct z_thpool_mng_struct *p_mng = (struct z_thpool_mng_struct *)handle;
    int32_t ret = -1;

    pthread_mutex_lock(&p_mng->mutex);
    if (!p_mng->start_flag) {
        goto error;
    }

    if (z_kfifo_space(&p_mng->t_info) < sizeof(struct z_thpool_msg_struct)) {
        goto error;
    }

    struct z_thpool_msg_struct msg;
    msg.cb = cb;
    msg.p_arg = p_arg;

    uint32_t len = z_kfifo_in(&p_mng->t_info, &msg, sizeof(struct z_thpool_msg_struct));
    if (len != sizeof(struct z_thpool_msg_struct)) {
        goto error;
    }

    p_mng->pub_bytes += len;
    pthread_cond_signal(&p_mng->cond);
    ret = 0;

error:
    pthread_mutex_unlock(&p_mng->mutex);
    return ret;
}

/**
@brief Display thread pool status
@param handle Handle to the thread pool
@return Status, success is 0
*/
int32_t z_thpool_cmd_shell_show(z_thpool_handle_t handle) {
    if (!handle) {
        return -EINVAL;
    }

    struct z_thpool_mng_struct *p_mng = (struct z_thpool_mng_struct *)handle;
    pthread_mutex_lock(&p_mng->mutex);

    z_table_print_title("z_thpool module");
    z_table_print_row("%-18s %s\n", "Ver: ", Z_THPOOL_VERION);
    z_table_print_row("%-18s %s\n", "Pool Name: ", p_mng->pool_name);
    z_table_print_border();
    z_table_print_row("%-18s %d\n", "max nums: ", p_mng->max_nums);
    z_table_print_row("%-18s %d\n", "create nums: ", p_mng->th_run_nums);
    z_table_print_row("%-18s %d\n", "busy nums:", p_mng->th_busy_nums);
    z_table_print_row("%-18s %d\n", "max cache nums:", p_mng->msg_node_max);
    z_table_print_row("%-18s %d\n", "use cache nums:", 
        z_kfifo_data_len(&p_mng->t_info) / sizeof(struct z_thpool_msg_struct));
    z_table_print_row("%-18s %d\n", "pub_bytes:", p_mng->pub_bytes);
    z_table_print_row("%-18s %d\n", "sub_bytes:", p_mng->sub_bytes);

    pthread_mutex_unlock(&p_mng->mutex);
    return 0;
}

/**
@brief Task callback function
@param p_arg Parameter
@return No return value
*/
static void z_thpool_test_task_cb(void *p_arg) {
    int32_t num = *((int32_t *)p_arg);
    printf("Task %d is being processed by thread %lu\n", num, pthread_self());
    sleep(1); // Simulate task processing time
}

/**
@brief Test the thread pool functionality
@return Status, success is 0
*/
int32_t z_thpool_test(void) {
    // Configuration for the thread pool extreme test
    struct z_thpool_config_struct t_config = {
        .max_thread_nums = 100,
        .msg_node_max = 100,
        .thread_stack_size = 64 * 1024
    };
    strncpy(t_config.pool_name, "test_pool", sizeof(t_config.pool_name) - 1);
    t_config.pool_name[sizeof(t_config.pool_name) - 1] = '\0';

    printf("Starting thread pool extreme test...\n");
    
    // Create thread pool
    z_thpool_handle_t handle;
    if (z_thpool_create(&t_config, &handle) != 0) {
        printf("Failed to create thread pool\n");
        return -1;
    }

    // Define task parameters array
    int32_t task_args[100];
    for (int32_t i = 0; i < 100; i++) {
        task_args[i] = i;
    }

    // Add tasks to the thread pool
    for (int32_t i = 0; i < 100; i++) {
        if (z_thpool_add_work(handle, z_thpool_test_task_cb, &task_args[i]) != 0) {
            fprintf(stderr, "Failed to add task %d\n", i);
        } else {
            printf("Task %d added successfully\n", i);
        }
    }

    // Wait for tasks to be processed
    for (int32_t i = 0; i < 5; i++) {
        printf("Waiting...\n");
        sleep(1);
    }

    // Verify data integrity
    for (int32_t i = 0; i < 100; i++) {
        printf("Task %d argument after processing: %d\n", i, task_args[i]);
    }

    // Show thread pool status
    z_thpool_cmd_shell_show(handle);

    // Destroy the thread pool
    z_thpool_destroy(handle);
    printf("Thread pool extreme test completed.\n");
    return 0;
}
