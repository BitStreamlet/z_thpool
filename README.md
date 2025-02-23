<div align="center">
  <img alt="z_thpool" width="120" height="120" src="./assets/logo/logo_1000.png">
  <h1>z_thpool</h1>
  <span>English | <a href="./README.zh-CN.md">中文</a></span>
</div>

<div align="center">
  <br/>
  <a href="" target="_blank"><img src="https://abroad.hellogithub.com/v1/widgets/recommend.svg?rid=9433615761f548cf9648434c670cd85b&claim_uid=249cPWvjfNmU7dp" alt="Featured｜HelloGitHub" style="width: 250px; height: 54px;" width="250" height="54" /></a>
</div>

## ⚡ Introduction

**This is a simple Linux thread pool library that supports multiple thread pool instances.** 😊

## 💻 Diagram
The internal structure consists of multiple thread pools, each with its own circular queue. When a thread pool is full, new tasks will be placed in its circular queue. Once a thread exits, it will fetch a task from the queue for processing.
<div align="center">
  <img alt="z_thpool" src="./assets/logic_block_diagram.png">
</div>

## 🚀 How to use?

**The following operations are based on the root directory of the current project, please ensure to perform them correctly!**

### **Compile**

```bash
cd z_thpool
make
cd lib
ls
libtestlib.a  libz_thpool.a   // Compilation completed
```

### **Test**

#### Complete Test
```bash
./build/z_thpool_test 
Enter a command (type 'exit' to quit):
> help  // Enter help to view test commands

Enter a command (type 'exit' to quit):

create pool1 5 100 64  # Create pool named 'pool1' with 5 threads, 100 cache queues, 64k stack size
destroy pool1         # Destroy pool named 'pool1'
add pool1 10         # Add 10 tasks to pool named 'pool1'
show pool1           # Show state of pool named 'pool1'
test                 # Run test suite
help                 # Show this help

> test  // Enter test to check complete test situation
Starting thread pool extreme test...
[debug ][src/z_thpool.c:182, z_thpool_create] enter
[debug ][src/z_thpool.c:229, z_thpool_create] exit :0
Task 0 added successfully
...
Task 99 added successfully
Waiting...
Task 0 is being processed by thread 140675616458496
...
Task 99 is being processed by thread 140675609564928
Waiting...
Waiting...
Waiting...
Waiting...
Task 0 argument after processing: 0
...
Task 99 argument after processing: 99
Thread pool extreme test completed.
```

#### Step-by-Step Test
```bash
./build/z_thpool_test 
Enter a command (type 'exit' to quit):
> create pool1 5 100 64    // Create a thread pool named 'pool1' with 5 threads, 100 cache queues, 64K stack size
Created thread pool 'pool1'

> create pool2 3 50 64     // Create another thread pool named 'pool2'
Created thread pool 'pool2'

> add pool1 10             // Add 10 tasks to pool1
Added task 0 to pool 'pool1'
...
Added task 9 to pool 'pool1'

> show pool1               // View pool1 state
------------------------------------------------------------------------------------------------------------------------
| z_thpool module                                                                                                                                          
------------------------------------------------------------------------------------------------------------------------
Ver:               0.0.2.0
Pool Name:         pool1
------------------------------------------------------------------------------------------------------------------------
max nums:          5     // Maximum threads is 5
create nums:       5     // Threads already created is 5
busy nums:         5     // Currently running threads is 5
max cache nums:    100   // Created cache queue is 100
use cache nums:    5     // Tasks waiting in the queue
pub_bytes:         200
sub_bytes:         0

> destroy pool1           // Destroy pool1
Destroyed thread pool 'pool1'
```

### **Usage Functions**
Just four steps to use the thread pool:
```c
// Step 1: Create a thread pool configuration
struct z_thpool_config_struct config;
config.max_thread_nums = 50;           // Maximum number of concurrently running threads is 50
config.msg_node_max = 1000;            // Maximum capacity of the cache queue is 1000 tasks
config.thread_stack_size = 64 * 1024;  // Stack size of each thread is 64KB
strncpy(config.pool_name, "pool1", sizeof(config.pool_name) - 1);  // Name your thread pool

// Step 2: Create the thread pool
z_thpool_handle_t handle;
if (z_thpool_create(&config, &handle) == 0) {
    printf("Thread pool created successfully\n");
}

// Step 3: Add tasks to the thread pool
void task_callback(void *arg) {
    printf("Processing task: %d\n", *(int*)arg);
}

int task_arg = 1;
if (z_thpool_add_work(handle, task_callback, &task_arg) == 0) {
    printf("Task added successfully\n");
}

// Step 4: When done, destroy the thread pool
z_thpool_destroy(handle);
```

### **Multiple Thread Pools Example**
```c
// Create two thread pools for different purposes
struct z_thpool_config_struct config1 = {
    .max_thread_nums = 5,
    .msg_node_max = 100,
    .thread_stack_size = 64 * 1024
};
strncpy(config1.pool_name, "io_pool", sizeof(config1.pool_name) - 1);

struct z_thpool_config_struct config2 = {
    .max_thread_nums = 10,
    .msg_node_max = 200,
    .thread_stack_size = 64 * 1024
};
strncpy(config2.pool_name, "compute_pool", sizeof(config2.pool_name) - 1);

z_thpool_handle_t io_pool, compute_pool;
z_thpool_create(&config1, &io_pool);
z_thpool_create(&config2, &compute_pool);

// Use different pools for different tasks
z_thpool_add_work(io_pool, io_task_callback, io_arg);
z_thpool_add_work(compute_pool, compute_task_callback, compute_arg);

// Clean up when done
z_thpool_destroy(io_pool);
z_thpool_destroy(compute_pool);
```

### **File Description**
The key files are:
```base
test.c          # Test program with command line interface
z_kfifo.c       # Cache queue implementation
z_thpool.c      # Thread pool implementation
z_debug.h       # Debug information toggle
z_tool.h        # Tool macros
z_table_print.c # Table printing utility
```

## 🛠️ Features
- Support for multiple thread pool instances
- Named thread pools for better management
- Independent configuration for each pool
- Thread-safe operations
- Resource cleanup on pool destruction
- Command-line interface for testing

## 🛠️ About

## ❓ FAQ

## 🤝 Development Guide 

## 🚀 Star 
[![Stargazers over time](https://starchart.cc/BitStreamlet/z_thpool.svg?variant=adaptive)](https://starchart.cc/BitStreamlet/z_thpool)

## 🌟 Contribution
Thanks to everyone who has contributed to z_thpool! 🎉
<a href="https://github.com//cuixueshe/earthworm/graphs/contributors"><img src="https://contributors.nn.ci/api?repo=BitStreamlet/z_thpool" /></a>

## 🌟 Acknowledgment
**Thank you for taking the time to read our project documentation.**
**If you find this project helpful, please support us with a Star. Thank you!**