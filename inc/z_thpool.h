#ifndef _Z_THPOOL_H_
#define _Z_THPOOL_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

// Handle type for thread pool instance
typedef struct z_thpool_mng_struct* z_thpool_handle_t;

// Data structure for configuring the thread pool
struct z_thpool_config_struct {
    uint32_t max_thread_nums;   // Maximum number of threads in the pool
    uint32_t msg_node_max;      // Maximum number of message nodes in the pool
    uint32_t thread_stack_size; // Stack size for each thread
    char pool_name[32];        // Name of the thread pool
};

// Function to create a new thread pool instance
// @param p_config: Pointer to a configuration structure specifying pool parameters
// @param p_handle: Pointer to store the created thread pool handle
// @return: Returns 0 on success, or a negative error code on failure
int32_t z_thpool_create(struct z_thpool_config_struct *p_config, z_thpool_handle_t *p_handle);

// Function to destroy a thread pool instance
// @param handle: Handle to the thread pool to destroy
// @return: Returns 0 on success, or a negative error code on failure
int32_t z_thpool_destroy(z_thpool_handle_t handle);

// Function to add a work task to the thread pool
// @param handle: Handle to the thread pool
// @param cb: The callback function that defines the task
// @param arg: The argument to pass to the task
// @return: Returns 0 on success, or a negative error code on failure
int32_t z_thpool_add_work(z_thpool_handle_t handle, void (*cb)(void *), void *arg);

// Function to get thread pool status
// @param handle: Handle to the thread pool
// @return: Returns 0 on success, or a negative error code on failure
int32_t z_thpool_cmd_shell_show(z_thpool_handle_t handle);

// Function to test the thread pool functionality
// @return: Returns 0 on success, or a negative error code on failure
int32_t z_thpool_test(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _Z_CONFIG_H_ */
