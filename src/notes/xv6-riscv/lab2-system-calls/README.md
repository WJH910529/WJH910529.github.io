---
title: "[LAB2] System calls"
date: 2026-07-11
category:
  - Notes
tag:
  - xv6
  - riscv
  - operating-system
  - lab
  - syscall
---

# [LAB2] System calls

原題目出處：[MIT 6.S081 / 6.1810 Lab: System calls](https://pdos.csail.mit.edu/6.S081/2022/labs/syscall.html)

這篇先整理 Lab2 裡的 **System call tracing**。這題的目標是在 xv6 新增一個 `trace(mask)` system call，讓 process 可以追蹤自己後續呼叫的 system calls。

目前完成狀態：System call tracing 的四個 grader tests 已通過。

## System call tracing

### 題目重點

新增一個 system call：

```c
int trace(int mask);
```

當 process 呼叫 `trace(mask)` 後，kernel 會記住這個 mask。之後這個 process 每次執行 system call 時，kernel 都會檢查該 system call 對應的 bit 是否有被打開。

如果有被打開，就在 system call 回到 user space 前印出：

```text
pid: syscall syscall-name -> return-value
```

例如：

```bash
trace 32 grep hello README
```

因為 `32 = 1 << SYS_read`，所以這個指令只追蹤 `read`：

```text
3: syscall read -> 1023
3: syscall read -> 961
3: syscall read -> 321
3: syscall read -> 0
```

這題還有兩個重要要求：

- trace 設定只影響呼叫它的 process，不能影響其他 process。
- 這個 process 之後 `fork()` 出來的 child 也要繼承相同 trace 設定。

## trace mask

### 題目重點

`kernel/syscall.h` 會替每個 system call 定義編號：

```c
#define SYS_fork   1
#define SYS_read   5
#define SYS_write 16
```

這些數字是 syscall number，也可以當成 mask 裡的 bit 位置。

要追蹤 `read`，不是使用 mask `5`，而是把第 5 個 bit 打開：

```c
1 << SYS_read
```

也就是：

```text
1 << 5 = 32
```

所以：

```bash
trace 32 grep hello README
```

代表只追蹤 `read`。

如果要同時追蹤多個 system calls，可以用 OR 把多個 bit 合起來：

```c
(1 << SYS_read) | (1 << SYS_write)
```

檢查某個 syscall 是否要被追蹤：

```c
if(mask & (1 << num)) {
  // num 對應的 bit 有被打開
}
```

`trace 2147483647` 則代表低 31 bits 幾乎都打開，可以追蹤 xv6 目前所有 system calls。

## 修改檔案

### 題目重點

新增 system call 不是只改 kernel 裡的一個函式，而是要把 user space 到 kernel space 的路徑全部接起來。

這次修改的檔案：

| 檔案 | 作用 |
|---|---|
| `Makefile` | 把 `_trace` user program 放進 xv6 file system |
| `user/user.h` | 宣告 `trace(int)` |
| `user/usys.pl` | 產生 user-space syscall stub |
| `kernel/syscall.h` | 分配 `SYS_trace` syscall number |
| `kernel/proc.h` | 在每個 process 裡保存 `trace_mask` |
| `kernel/proc.c` | 初始化 mask，並在 `fork()` 時複製給 child |
| `kernel/sysproc.c` | 實作 `sys_trace()` |
| `kernel/syscall.c` | 註冊 handler、建立名稱表、輸出 tracing 結果 |

整體路徑可以想成：

```text
user/trace.c
    -> user/user.h
    -> user/usys.pl
    -> user/usys.S
    -> kernel/syscall.h
    -> kernel/syscall.c
    -> kernel/sysproc.c
    -> struct proc
```

## 我的解法

### 1. 加入 trace user program

在 `Makefile` 的 `UPROGS` 加入：

```makefile
$U/_trace\
```

這樣 xv6 build 時才會把課程提供的 `user/trace.c` 編成可執行檔，並放進 xv6 的 file system image。

### 2. 宣告 user-space trace()

在 `user/user.h` 加入：

```c
int trace(int);
```

這個宣告只負責讓 compiler 知道 `trace()` 的函式型別，真正進入 kernel 的 stub 還要靠 `user/usys.pl` 產生。

### 3. 產生 syscall stub

在 `user/usys.pl` 加入：

```perl
entry("trace");
```

`usys.pl` 會產生 `user/usys.S`，概念上等於：

```asm
.global trace
trace:
  li a7, SYS_trace
  ecall
  ret
```

重點是：

- `a7` 放 syscall number。
- `a0` 放第一個參數，也就是 `mask`。
- `ecall` 讓 CPU 從 user mode trap 進 kernel。

### 4. 分配 SYS_trace

在 `kernel/syscall.h` 加入：

```c
#define SYS_trace 22
```

這裡的 `22` 是 `trace` 這個 system call 的編號。

要注意：

```text
SYS_trace = 22
trace 32  = 追蹤 SYS_read
```

這兩個數字的用途不同。`22` 是 `trace` 自己的 syscall number，`32` 是用來追蹤 `read` 的 mask。

### 5. 在 struct proc 保存 trace_mask

因為 trace 設定是 per-process state，所以我把它放在 `struct proc`。

在 `kernel/proc.h` 的 `struct proc` 加入：

```c
int trace_mask;
```

這樣每個 process 都有自己的追蹤設定：

```text
process A: trace_mask = 32
process B: trace_mask = 0
process C: trace_mask = 2147483647
```

### 6. 初始化 trace_mask

在 `kernel/proc.c` 的 `allocproc()` 初始化：

```c
p->trace_mask = 0;
```

新 process 預設不追蹤任何 system call。這也可以避免重用 `struct proc` 時拿到上一個 process 留下的 mask。

### 7. 實作 sys_trace()

在 `kernel/sysproc.c` 加入：

```c
uint64
sys_trace(void)
{
  int mask;

  argint(0, &mask);
  myproc()->trace_mask = mask;
  return 0;
}
```

這裡的 `argint(0, &mask)` 會從目前 process 的 trapframe 取出第一個 user argument。

流程大概是：

```text
trace(32)
    -> a0 = 32
    -> ecall
    -> trapframe->a0 = 32
    -> argint(0, &mask)
    -> mask = 32
```

`sys_trace()` 只負責保存 mask，不負責印出 tracing 結果。

### 8. 註冊 sys_trace()

在 `kernel/syscall.c` 加入 handler 宣告：

```c
extern uint64 sys_trace(void);
```

並加入 syscall dispatch table：

```c
static uint64 (*syscalls[])(void) = {
  // ...
  [SYS_trace] sys_trace,
};
```

這樣 `syscall()` 讀到 `a7 = SYS_trace` 時，才知道要呼叫 `sys_trace()`。

### 9. 建立 syscall name table

題目要求輸出 system call 名稱，所以我在 `kernel/syscall.c` 建立名稱表：

```c
static char *syscall_names[] = {
  [SYS_fork]   = "fork",
  [SYS_exit]   = "exit",
  [SYS_wait]   = "wait",
  [SYS_pipe]   = "pipe",
  [SYS_read]   = "read",
  [SYS_kill]   = "kill",
  [SYS_exec]   = "exec",
  [SYS_fstat]  = "fstat",
  [SYS_chdir]  = "chdir",
  [SYS_dup]    = "dup",
  [SYS_getpid] = "getpid",
  [SYS_sbrk]   = "sbrk",
  [SYS_sleep]  = "sleep",
  [SYS_uptime] = "uptime",
  [SYS_open]   = "open",
  [SYS_write]  = "write",
  [SYS_mknod]  = "mknod",
  [SYS_unlink] = "unlink",
  [SYS_link]   = "link",
  [SYS_mkdir]  = "mkdir",
  [SYS_close]  = "close",
  [SYS_trace]  = "trace",
};
```

我使用 designated initializer，讓 index 直接等於 syscall number：

```text
syscall_names[SYS_read] = "read"
```

這樣比用 `syscall_names[num - 1]` 安全，因為就算 syscall number 中間有空洞，也不會讓名稱對錯位置。

### 10. 在 syscall() 印出 tracing 結果

在 `kernel/syscall.c` 的 `syscall()` 裡，原本會呼叫真正的 handler，並把 return value 放回 `a0`：

```c
p->trapframe->a0 = syscalls[num]();
```

我在這之後加入 mask 檢查：

```c
if(p->trace_mask & (1 << num)) {
  printf("%d: syscall %s -> %d\n",
         p->pid, syscall_names[num], (int)p->trapframe->a0);
}
```

完整概念：

```c
if(num > 0 && num < NELEM(syscalls) && syscalls[num]) {
  p->trapframe->a0 = syscalls[num]();

  if(p->trace_mask & (1 << num)) {
    printf("%d: syscall %s -> %d\n",
           p->pid, syscall_names[num], (int)p->trapframe->a0);
  }
} else {
  printf("%d %s: unknown sys call %d\n", p->pid, p->name, num);
  p->trapframe->a0 = -1;
}
```

順序很重要：一定要先執行 handler，取得 return value 後再印。否則 `a0` 可能還是原本的 argument，不是 system call 的回傳值。

### 11. fork() 複製 trace_mask

`exec()` 不用特別處理，因為它會替換目前 process 的 user program，但保留同一個 `struct proc`。

`fork()` 會建立新的 child `struct proc`，所以要手動複製：

```c
// copy saved user registers.
*(np->trapframe) = *(p->trapframe);

// Copy trace settings from parent to child.
np->trace_mask = p->trace_mask;

// Cause fork to return 0 in the child.
np->trapframe->a0 = 0;
```

概念上：

```text
parent p->trace_mask = 2
          |
          v fork()
child np->trace_mask = 2
```

## 解法說明

`trace(mask)` 本身不負責追蹤所有 system call。它只做一件事：把 mask 存進目前 process 的 `struct proc`。

真正負責輸出 tracing line 的地方是 `syscall()`：

```text
user program 呼叫 read()
    -> user stub 把 SYS_read 放進 a7
    -> ecall 進 kernel
    -> syscall() 呼叫 sys_read()
    -> return value 放進 trapframe->a0
    -> 檢查 trace_mask 的第 SYS_read bit
    -> 如果 bit 有打開，就印出 tracing line
```

這題最重要的觀念是：

- system call number 放在 `a7`。
- system call arguments 放在 `a0`、`a1`、`a2` 等 register。
- trap 進 kernel 後，xv6 透過 trapframe 取回這些 register。
- return value 會被放回 `trapframe->a0`。
- per-process 的狀態應該放在 `struct proc`。

## 測試

只追蹤 `read`：

```bash
trace 32 grep hello README
```

預期只看到 `read`：

```text
3: syscall read -> 1023
3: syscall read -> 961
3: syscall read -> 321
3: syscall read -> 0
```

追蹤所有 system calls：

```bash
trace 2147483647 grep hello README
```

會看到 `trace`、`exec`、`open`、`read`、`close` 等 system calls：

```text
4: syscall trace -> 0
4: syscall exec -> 3
4: syscall open -> 3
4: syscall read -> 1023
4: syscall read -> 961
4: syscall read -> 321
4: syscall read -> 0
4: syscall close -> 0
```

未啟用 tracing：

```bash
grep hello README
```

不應該印出任何 tracing line。這可以確認 `trace_mask` 沒有錯誤地影響其他 process。

驗證 child inheritance：

```bash
trace 2 usertests forkforkfork
```

`2 = 1 << SYS_fork`，所以只追蹤 `fork`。parent 和 descendants 都應該印出 fork tracing line。

最後執行 grader：

```bash
make grade GRADEFLAGS=trace
```

本次 System call tracing 測試結果：

```text
trace 32 grep: OK
trace all grep: OK
trace nothing: OK
trace children: OK
```

## 小結

這題練到的是新增 system call 的完整路徑：

```text
user declaration
-> user syscall stub
-> syscall number
-> kernel handler
-> kernel dispatch table
-> per-process state
```

也更清楚看出 `fork()` 和 `exec()` 的差異：

- `exec()`：換掉 user program，但保留同一個 `struct proc`。
- `fork()`：建立新的 child `struct proc`，需要手動複製要繼承的狀態。

`trace_mask` 也示範了 bit mask 的典型用途：用一個 integer 的不同 bits 表示多個開關，適合拿來表示 flags、permissions 或這題的 syscall tracing 設定。

## 後續待補

Lab2 還包含 `Sysinfo` 與一些 gdb / answers 題目。這篇先整理已完成的 System call tracing，後續完成 `Sysinfo` 後再補下一篇或更新本篇。
