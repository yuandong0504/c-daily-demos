# C Daily Demos

这是我的 C 语言每日练习仓库，用来积累指针、结构体、信号、网络编程等底层知识的代码片段。  
目标是长期沉淀，形成一个 **知识宝库**。

---

## 📂 目录索引

### 🔹 基础入门
- `hello.c` — Hello World 示例
- `add_one.c` — 简单函数调用与返回
- `double_all.c` — 数组元素翻倍
- `print_matrix.c` — 打印二维矩阵

### 🔹 指针与内存
- `arr2d_demo.c` — 二维数组与指针
- `by_value_pointer.c` — 值传递 vs 指针传递
- `double_ptr_demo.c` — 双重指针演示
- `f_pointer.c` — 函数指针示例
- `p_reverse.c` — 使用指针反转字符串
- `reverse_str.c` — 字符串反转（指针版本）
- `squeeze_str.c` — 压缩字符串

### 🔹 结构体
- `struct_student.c` — 学生信息 struct 示例
- `struct_ptr.c` — struct 与指针结合

### 🔹 算法与数据结构
- `bubble_sort.c` — 冒泡排序
- `push_pop_show.c` — 栈的 push/pop 演示

### 🔹 系统编程（信号 / 进程）
- `signal.c` — 捕捉 SIGINT 信号
- `alarm.c` — 使用 `alarm()` + 信号实现超时退出
- `lamp.c` — 小实验（进程/系统调用）

### 🔹 待扩展（网络 / eBPF / WASM）
- **预留**：socket demo, epoll demo, eBPF trace demo, WASM host demo …

---

## 🚀 使用方法

编译与运行示例：
```bash
gcc hello.c -o hello
./hello
```

---

## 🧭 学习主线

- [x] 指针 (pointer)  
- [x] struct (结构体)  
- [x] signal (信号/异步编程)  
- [ ] 网络编程 (socket / TCP / UDP)  
- [ ] eBPF 基础 (bpftool, map, tracepoint)  
- [ ] WASM 实验 (wasmtime/wasmer + C API)  

---

## 📅 进度记录

- 2025-09-27：从 iSH 导入 `arr2d_demo.c`, `double_all.c`, `hello.c`, `print_matrix.c`  
- 2025-09-27：完成 GitHub SSH 配置，全部 demo 同步到云端  

---
