# 进程同步与并发控制

本实验使用 C++ 多线程实现 3 个经典同步问题：

- 生产者 - 消费者问题
- 读者 - 写者问题
- 哲学家进餐问题

程序重点演示：

- 线程并发执行
- `mutex` 与 `condition_variable` 的使用
- 如何避免竞态条件和死锁

## 编译

```bash
cd 03_process_synchronization
g++ -std=c++17 -O2 -Wall -Wextra -pedantic main.cpp -o synchronization -pthread
```

或使用 CMake：

```bash
cd 03_process_synchronization
cmake -S . -B build
cmake --build build
```

## 运行

```bash
./synchronization
```

## 菜单

- `1`：生产者 - 消费者
- `2`：读者 - 写者
- `3`：哲学家进餐
- `4`：依次运行全部示例
- `0`：退出

## 输出说明

程序会打印线程执行日志，例如：

- 哪个生产者放入了哪个数据
- 哪个消费者取出了哪个数据
- 哪个读者读取了共享数据
- 哪个写者更新了共享数据
- 哪个哲学家正在思考、拿叉子、吃饭

由于并发调度具有随机性，所以每次运行日志顺序可能略有不同，这是正常现象。
