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

這篇先整理 Lab2 裡的 **System call tracing**。這題的目標是在 xv6 裡新增一個 `trace(mask)` system call，讓指定 process 可以追蹤自己後續呼叫的 system calls。

完成狀態：System call tracing 的 grader tests 已通過。

## System call tracing

### 題目重點

要新增一個 system call：

```c
int trace(int mask);
```

當 process 呼叫 `trace(mask)` 後，kernel 要記住這個 mask。之後這個 process 每次執行 system call 時，kernel 都要檢查該 system call 對應的 bit 是否被打開。

如果有被打開，就在 system call 即將返回 user space 前印出：

```text
pid: syscall syscall-name -> return-value
```

例如：

```bash
trace 32 grep hello README
```

其中 `32 = 1 << SYS_read`，所以只追蹤 `read`：

```text
3: syscall read -> 1023
3: syscall read -> 961
3: syscall read -> 321
3: syscall read -> 0
```

題目還要求：

- trace 設定只影響呼叫它的 process。
- 不可以影響其他 process。
- 之後 `fork()` 出來的 child process 要繼承相同 trace 設定。

## trace mask

### Syscall number 和 mask 不一樣

`kernel/syscall.h` 會替每個 system call 定義編號：

```c
#define SYS_fork   1
#define SYS_read   5
#define SYS_write 16
```

這些數字是 syscall number，也是 bit mask 裡對應的 bit 位置。

如果要追蹤 `read`，不是使用 mask `5`，而是要把第 5 個 bit 打開：

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

### 同時追蹤多個 system calls

bit mask 可以同時打開多個 bit。例如同時追蹤 `read` 和 `write`：

```c
(1 << SYS_read) | (1 << SYS_write)
```

檢查某個 system call 是否要被追蹤時，用：

```c
if(mask & (1 << num)) {
  // num 對應的 bit 有被打開
}
```

`trace 2147483647` 則代表低 31 bits 幾乎都打開，因此可以追蹤 xv6 目前所有 system calls。

## 新增 system call 的路徑

新增 `trace(mask)` 不是只改一個地方，而是要把 user space 到 kernel space 的路徑全部接起來。

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

## user/trace.c 在做什麼

`user/trace.c` 是課程已經提供的 user program。它的用途是先呼叫 `trace(mask)`，再用 `exec()` 執行真正要追蹤的 command。

核心流程：

```text
trace 32 grep hello README
```

arguments 會長這樣：

```text
argv[0] = "trace"
argv[1] = "32"
argv[2] = "grep"
argv[3] = "hello"
argv[4] = "README"
```

程式會先做：

```c
trace(atoi(argv[1]));
```

也就是把目前 process 的 trace mask 設成 `32`。

接著整理 command：

```text
nargv[0] = "grep"
nargv[1] = "hello"
nargv[2] = "README"
```

最後呼叫：

```c
exec(nargv[0], nargv);
```

`exec()` 會替換目前 process 的 user program，但不會換掉 `struct proc`。所以 `trace_mask` 仍然留在同一個 process 裡：

```text
trace program
pid = 3, trace_mask = 32
        |
        v exec("grep", ...)
grep program
pid = 3, trace_mask = 32
```

## 我的解法

### 1. 把 trace program 加入 Makefile

在 `UPROGS` 加入：

```makefile
$U/_trace\
```

這樣 xv6 build 時才會把 `user/trace.c` 編成可執行檔，並放進 xv6 的 file system image。

### 2. 宣告 user-space trace()

在 `user/user.h` 加入：

```c
int trace(int);
```

這只是讓 compiler 知道 `trace()` 的函式型別，還不是真正的 system call 實作。

如果少了這行，會遇到：

```text
implicit declaration of function 'trace'
```

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

如果少了這步，linker 會找不到 `trace` symbol：

```text
undefined reference to `trace'
```

### 4. 分配 SYS_trace

在 `kernel/syscall.h` 加入：

```c
#define SYS_trace 22
```

這裡的 `22` 是 syscall number，用來告訴 kernel 這次 `ecall` 要呼叫 `trace`。

要注意：

```text
SYS_trace = 22
trace mask = 32
```

這兩個數字意思不同。`22` 是 `trace` 自己的 syscall number，`32` 是用來追蹤 `read` 的 mask。

### 5. 在 struct proc 保存 trace_mask

因為 trace 設定要屬於個別 process，所以不能用 global variable。正確位置是 `struct proc`。

在 `kernel/proc.h` 的 `struct proc` 加入：

```c
int trace_mask;
```

這樣每個 process 都有自己的設定：

```text
process A: trace_mask = 32
process B: trace_mask = 0
process C: trace_mask = 2147483647
```

### 6. 初始化 trace_mask

在 `kernel/proc.c` 的 `allocproc()` 裡初始化：

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

`argint(0, &mask)` 會從目前 process 的 trapframe 取出第一個 user argument，也就是 `a0` 裡的值。

流程大概是：

```text
trace(32)
    -> a0 = 32
    -> ecall
    -> trapframe->a0 = 32
    -> argint(0, &mask)
    -> mask = 32
```

`sys_trace()` 本身只負責保存 mask，不負責印出 tracing 結果。

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

如果 user stub 已經成功進 kernel，但這裡沒有註冊，會看到：

```text
unknown sys call 22
```

### 9. 建立 syscall name table

題目要求輸出 syscall 名稱，所以在 `kernel/syscall.c` 建立名稱表：

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

我選擇用 designated initializer，讓 index 直接等於 syscall number：

```text
syscall_names[SYS_read] = "read"
```

不要用 `syscall_names[num - 1]`，因為如果 syscall number 中間有空洞，名稱就可能對錯位置。

### 10. 在 syscall() 輸出 tracing 結果

在 `kernel/syscall.c` 的 `syscall()` 裡，原本會呼叫真正的 handler，並把 return value 放回 `a0`：

```c
p->trapframe->a0 = syscalls[num]();
```

在這之後加入 mask 檢查：

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

順序很重要：一定要先執行 handler，拿到 return value 之後再印。否則 `a0` 裡可能還是原本的 argument，而不是 system call 的回傳值。

### 11. fork() 複製 trace_mask

題目要求 child process 也要繼承 tracing 設定。

`exec()` 不用特別處理，因為它還是同一個 `struct proc`。

但 `fork()` 會建立新的 child `struct proc`，所以要手動複製：

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

如果少了這行，`trace children` 測試會失敗。

## 測試結果

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

## 常見錯誤

### implicit declaration of function 'trace'

原因是 `user/user.h` 沒有宣告：

```c
int trace(int);
```

這是 compiler 階段錯誤。

### undefined reference to `trace`

原因是 `user/usys.pl` 沒有加入：

```perl
entry("trace");
```

這是 linker 階段錯誤。

### unknown sys call 22

代表 user stub 已經成功進 kernel，但 kernel dispatch table 還沒有註冊：

```c
[SYS_trace] sys_trace,
```

### trace 成功，但沒有任何 tracing output

通常是 `sys_trace()` 已經保存 mask，但 `syscall()` 還沒有加入：

```c
if(p->trace_mask & (1 << num))
```

### trace children 測試失敗

通常是 `fork()` 沒有複製：

```c
np->trace_mask = p->trace_mask;
```

### return value 印出錯誤

記得要印 `p->trapframe->a0`，因為 system call handler 的 return value 會被放回這裡：

```c
(int)p->trapframe->a0
```

## 小結

這題真正練到的是「新增一個 system call 的完整路徑」：

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
