<div align="center">
  <img alt="z_thpool" width="120" height="120" src="./assets/logo/logo_1000.png">
  <h1>z_thpool</h1>
  <span><a href="./README.md">English</a> | 中文</span>
</div>

<div align="center">
  <br/>
  <a href="" target="_blank"><img src="https://abroad.hellogithub.com/v1/widgets/recommend.svg?rid=9433615761f548cf9648434c670cd85b&claim_uid=249cPWvjfNmU7dp" alt="Featured｜HelloGitHub" style="width: 250px; height: 54px;" width="250" height="54" /></a>
</div>

## ⚡ 简介

**这是一个支持多实例的简单 Linux 线程池库。** 😊

## 💻 架构图
内部结构由多个线程池组成，每个线程池都有自己的循环队列。当线程池满时，新任务将被放入对应的循环队列中。当线程完成任务后，会从队列中获取新任务进行处理。
<div align="center">
  <img alt="z_thpool" src="./assets/logic_block_diagram.png">
</div>

## 🚀 如何使用？

**以下操作都基于项目根目录，请确保正确执行！**

### **编译**

```bash
cd z_thpool
make
cd lib
ls
libtestlib.a  libz_thpool.a   // 编译完成
```

### **测试**

#### 完整测试
```bash
./build/z_thpool_test 
输入命令（输入 'exit' 退出）：
> help  // 输入 help 查看测试命令

输入命令（输入 'exit' 退出）：

create pool1 5 100 64  # 创建名为 'pool1' 的线程池，5个线程，100个缓存队列，64k栈大小
destroy pool1         # 销毁名为 'pool1' 的线程池
add pool1 10         # 向 'pool1' 添加10个任务
show pool1           # 显示 'pool1' 的状态
test                 # 运行测试套件
help                 # 显示帮助信息

> test  // 输入 test 检查完整测试情况
开始线程池极限测试...
[debug ][src/z_thpool.c:182, z_thpool_create] enter
[debug ][src/z_thpool.c:229, z_thpool_create] exit :0
任务 0 添加成功
...
任务 99 添加成功
等待中...
任务 0 正在被线程 140675616458496 处理
...
任务 99 正在被线程 140675609564928 处理
等待中...
等待中...
等待中...
等待中...
任务 0 处理后的参数: 0
...
任务 99 处理后的参数: 99
线程池极限测试完成。
```

#### 逐步测试
```bash
./build/z_thpool_test 
输入命令（输入 'exit' 退出）：
> create pool1 5 100 64    // 创建名为 'pool1' 的线程池，5个线程，100个缓存队列，64K栈大小
线程池 'pool1' 创建成功

> create pool2 3 50 64     // 创建另一个名为 'pool2' 的线程池
线程池 'pool2' 创建成功

> add pool1 10             // 向pool1添加10个任务
向线程池 'pool1' 添加任务 0
...
向线程池 'pool1' 添加任务 9

> show pool1               // 查看pool1状态
------------------------------------------------------------------------------------------------------------------------
| z_thpool 模块                                                                                                                                          
------------------------------------------------------------------------------------------------------------------------
版本:               0.0.2.0
线程池名称:         pool1
------------------------------------------------------------------------------------------------------------------------
最大线程数:         5     // 最大线程数为5
已创建线程数:       5     // 已创建5个线程
忙碌线程数:         5     // 当前有5个线程正在运行
最大缓存数:         100   // 创建了100个缓存队列
已用缓存数:         5     // 队列中等待的任务数
发布字节数:         200
订阅字节数:         0

> destroy pool1           // 销毁pool1
线程池 'pool1' 已销毁
```

### **使用方法**
使用线程池只需四个步骤：
```c
// 步骤1：创建线程池配置
struct z_thpool_config_struct config;
config.max_thread_nums = 50;           // 最大并发运行线程数为50
config.msg_node_max = 1000;            // 缓存队列最大容量为1000个任务
config.thread_stack_size = 64 * 1024;  // 每个线程的栈大小为64KB
strncpy(config.pool_name, "pool1", sizeof(config.pool_name) - 1);  // 为线程池命名

// 步骤2：创建线程池
z_thpool_handle_t handle;
if (z_thpool_create(&config, &handle) == 0) {
    printf("线程池创建成功\n");
}

// 步骤3：添加任务到线程池
void task_callback(void *arg) {
    printf("正在处理任务: %d\n", *(int*)arg);
}

int task_arg = 1;
if (z_thpool_add_work(handle, task_callback, &task_arg) == 0) {
    printf("任务添加成功\n");
}

// 步骤4：完成后，销毁线程池
z_thpool_destroy(handle);
```

### **多线程池示例**
```c
// 创建两个用于不同目的的线程池
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

// 使用不同的池处理不同的任务
z_thpool_add_work(io_pool, io_task_callback, io_arg);
z_thpool_add_work(compute_pool, compute_task_callback, compute_arg);

// 完成后清理
z_thpool_destroy(io_pool);
z_thpool_destroy(compute_pool);
```

### **文件说明**
主要文件包括：
```base
test.c          # 带命令行界面的测试程序
z_kfifo.c       # 循环队列实现
z_thpool.c      # 线程池实现
z_debug.h       # 调试信息开关
z_tool.h        # 工具宏
z_table_print.c # 表格打印工具
```

## 🛠️ 特性
- 支持多线程池实例
- 命名线程池便于管理
- 每个池独立配置
- 线程安全操作
- 资源自动清理
- 命令行测试接口

## 🛠️ 关于

## ❓ 常见问题

## 🤝 开发指南

## 🚀 Star趋势
[![Stargazers over time](https://starchart.cc/BitStreamlet/z_thpool.svg?variant=adaptive)](https://starchart.cc/BitStreamlet/z_thpool)

## 🌟 贡献者
感谢所有为 z_thpool 做出贡献的人！🎉
<a href="https://github.com//cuixueshe/earthworm/graphs/contributors"><img src="https://contributors.nn.ci/api?repo=BitStreamlet/z_thpool" /></a>

## 🌟 致谢
**感谢您花时间阅读我们的项目文档。**
**如果您觉得这个项目有帮助，请给我们一个 Star。谢谢！**
