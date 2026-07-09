---
title: "[LAB1] Xv6 and Unix utilities"
date: 2026-07-08
category:
  - Notes
tag:
  - xv6
  - riscv
  - operating-system
  - lab
---

# [LAB1] Xv6 and Unix utilities

原題目出處：[MIT 6.S081 / 6.1810 Lab: Xv6 and Unix utilities](https://pdos.csail.mit.edu/6.S081/2022/labs/util.html)

這個 lab 的目標是熟悉 xv6 的使用者程式、system call、pipe、fork、exec，以及簡單的 Unix utility 實作方式。

## Boot xv6

### 題目重點

先把 xv6-labs-2022 複製下來，進入 lab 對應的分支後，用 QEMU 啟動 xv6。

```bash
git clone git://g.csail.mit.edu/xv6-labs-2022
cd xv6-labs-2022
make qemu
```

如果是在 Windows 上操作，我目前主要透過 WSL 執行 xv6 環境。WSL 可以讓 Windows 直接使用 Linux shell、compiler 和開發工具，不用另外開完整虛擬機。

離開 QEMU：

```text
Ctrl + A
放開後按 X
```

## sleep

### 題目重點

實作一個 xv6 使用者程式 `sleep`，讓程式暫停指定的 ticks 數量。ticks 是 xv6 kernel 裡的時間單位，約略可以理解成 timer interrupt 之間的間隔。

目標檔案：

```text
user/sleep.c
```

並且要把程式加入 `Makefile` 的 `UPROGS`：

```makefile
$U/_sleep\
```

### 我的解法

```c
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  if(argc != 2){
    printf("the argument number input error");
  }

  int tick = atoi(argv[1]);
  sleep(tick);

  exit(0);
}
```

### 解法說明

這題的核心很單純：

- 用 `argc` 檢查使用者是否有傳入 ticks。
- 用 `atoi(argv[1])` 把字串轉成整數。
- 呼叫 xv6 已經提供的 `sleep(tick)` system call。
- 最後用 `exit(0)` 結束程式。

比較需要注意的是，若參數數量錯誤，印出錯誤訊息後最好直接 `exit(1)`，不然程式仍會繼續讀取 `argv[1]`。

當使用者程式呼叫 `sleep(tick)` 時，大致流程是：

```text
user space 呼叫 sleep
-> 透過 ecall 進入 kernel
-> uservec / usertrap 處理 trap
-> syscall() 根據 syscall number 找到 sys_sleep()
-> process 進入 SLEEPING
-> timer interrupt 累積足夠 ticks 後被喚醒
```

測試：

```bash
./grade-lab-util sleep
```

## pingpong

### 題目重點

使用 `pipe` 和 `fork` 建立 parent / child 兩個 process，讓它們互相傳一個 byte。

輸出格式大致如下：

```text
<child pid>: received ping
<parent pid>: received pong
```

目標檔案：

```text
user/pingpong.c
```

並且加入 `Makefile`：

```makefile
$U/_pingpong\
```

### 我的解法

```c
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main()
{
  int p1[2], p2[2];
  char buf[1];

  pipe(p1);
  pipe(p2);

  if(fork() == 0){
    read(p1[0], buf, 1);
    printf("%d: received ping\n", getpid());
    write(p2[1], buf, 1);
    exit(0);
  } else {
    write(p1[1], "a", 1);
    read(p2[0], buf, 1);
    printf("%d: received pong\n", getpid());
    wait(0);
  }

  exit(0);
}
```

### 解法說明

這題用兩條 pipe：

- `p1`：parent 寫，child 讀。
- `p2`：child 寫，parent 讀。

流程是 parent 先送一個 byte 給 child，child 讀到後印出 `ping`，再把 byte 傳回 parent，parent 收到後印出 `pong`。

測試：

```bash
./grade-lab-util pingpong
```

## primes

### 題目重點

用 pipe 和 fork 實作 concurrent prime sieve。主程式先送出 `2` 到 `35`，每一層 process 讀到的第一個數字就是 prime，接著把不能被該 prime 整除的數字傳給下一層 process。

目標檔案：

```text
user/primes.c
```

並且加入 `Makefile`：

```makefile
$U/_primes\
```

### 我的解法

```c
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

void
sieve(int left_pipe[2])
{
  int prime;

  if(read(left_pipe[0], &prime, sizeof(prime)) == 0){
    close(left_pipe[0]);
    return;
  }

  printf("prime %d\n", prime);

  int right_pipe[2];
  pipe(right_pipe);

  int pid = fork();
  if(pid == 0){
    close(right_pipe[1]);
    sieve(right_pipe);
    close(right_pipe[0]);
    exit(0);
  } else {
    close(right_pipe[0]);

    int num;
    while(read(left_pipe[0], &num, sizeof(num)) > 0){
      if(num % prime != 0){
        write(right_pipe[1], &num, sizeof(num));
      }
    }

    close(left_pipe[0]);
    close(right_pipe[1]);
    wait(0);
    exit(0);
  }
}

int
main()
{
  int initial_pipe[2];
  pipe(initial_pipe);

  int pid = fork();

  if(pid == 0){
    close(initial_pipe[1]);
    sieve(initial_pipe);
  } else {
    close(initial_pipe[0]);

    for(int i = 2; i <= 35; i++){
      write(initial_pipe[1], &i, sizeof(i));
    }

    close(initial_pipe[1]);
    wait(0);
    exit(0);
  }
}
```

### 解法說明

這題的關鍵是把每個 prime 當成一個 filter：

- 每層 process 先讀到第一個數字，這個數字就是 prime。
- 建立右邊的 pipe，並 fork 下一層 process。
- 目前這層繼續從左邊 pipe 讀數字。
- 如果數字不能被目前的 prime 整除，就寫到右邊 pipe。
- 當左邊 pipe 沒資料後，要關閉不需要的 file descriptor，讓下一層能正確結束。

這題最容易出錯的是忘記 `close()`。如果 write end 沒有被關掉，下一層的 `read()` 可能會一直等不到 EOF。

## find

### 題目重點

實作簡化版 Unix `find`，在目錄樹中尋找指定檔名。

目標檔案：

```text
user/find.c
```

題目提示的方向：

- 參考 `user/ls.c` 讀目錄的方式。
- 用 recursion 進入子目錄。
- 不要遞迴進入 `.` 和 `..`。
- 字串比較要用 `strcmp()`，不能用 `==`。

### 我的解法

```c
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

/*
find(path, target_name) quick review:

1) open(path) + fstat(fd, &st):
   - fstat is for an already-open fd.
   - Used to determine what "current path" is (T_DIR/T_FILE/T_DEVICE).

2) If current path is a directory (T_DIR):
   - read each dirent from fd
   - build child path into buf: "<path>/<entry>"
   - stat(buf, &st) to inspect each child entry type
     (stat is for a path string, not an fd)

3) For each child:
   - skip "." and ".." to avoid infinite recursion
   - if child is T_DIR: recurse find(buf, target_name)
   - if child is T_FILE and name matches: print full path

4) Close fd before return in all paths.
*/

void
find(char *path, char *target_name)
{
  char buf[512], *p;
  int fd;
  struct dirent de;
  struct stat st;

  if((fd = open(path, 0)) < 0){
    fprintf(2, "find: cannot open %s\n", path);
    return;
  }

  if(fstat(fd, &st) < 0){
    fprintf(2, "find: cannot stat %s\n", path);
    close(fd);
    return;
  }

  switch(st.type){
  case T_DEVICE:
  case T_FILE:
    break;

  case T_DIR:
    if(strlen(path) + 1 + DIRSIZ + 1 > sizeof buf){
      printf("find: path too long\n");
      break;
    }
    strcpy(buf, path);
    p = buf+strlen(buf);
    *p++ = '/';
    while(read(fd, &de, sizeof(de)) == sizeof(de)){
      if(de.inum == 0)
        continue;
      memmove(p, de.name, DIRSIZ);
      p[DIRSIZ] = 0;
      if(stat(buf, &st) < 0){
        printf("find: cannot stat %s\n", buf);
        continue;
      }

      if(strcmp(de.name, ".") != 0 && strcmp(de.name, "..") != 0){
        if(st.type == T_DIR)
          find(buf, target_name);
        else if(st.type == T_FILE)
          if(strcmp(de.name, target_name) == 0)
            printf("%s\n", buf);
      }
    }
    break;
  }
  close(fd);
}

int
main(int argc, char *argv[])
{
  if(argc == 3){
    find(argv[1], argv[2]);
    exit(0);
  }

  printf("Usage: find <path> <target_name>\n");
  exit(0);
}
```

### 解法說明

這題的核心是「走訪目錄樹」：

- `open(path, 0)` 打開目前路徑。
- `fstat(fd, &st)` 判斷目前打開的 fd 是 `T_DIR`、`T_FILE` 還是 `T_DEVICE`。
- 如果目前是目錄，就用 `read(fd, &de, sizeof(de))` 逐一讀出 `struct dirent`。
- 把目前路徑和目錄項目組成下一層路徑：`<path>/<entry>`。
- 用 `stat(buf, &st)` 判斷子項目是檔案還是目錄。
- 如果是目錄，就遞迴呼叫 `find(buf, target_name)`。
- 如果是檔案，而且 `strcmp(de.name, target_name) == 0`，就印出完整路徑。

這裡有幾個重要細節：

- `fstat()` 是用在已經打開的 file descriptor。
- `stat()` 是用在路徑字串。
- 一定要跳過 `.` 和 `..`，不然遞迴會一直繞回自己或父目錄。
- `de.name` 是字元陣列，字串比較要用 `strcmp()`，不能用 `==`。
- 函式結束前要 `close(fd)`，避免 file descriptor 沒有釋放。

## xargs

### 題目重點

實作簡化版 Unix `xargs`：從標準輸入讀取每一行，將讀到的內容附加到指定 command 的參數後面，然後 fork 子 process 執行該 command。

例如：

```bash
echo hello too | xargs echo bye
```

概念上會執行：

```bash
echo bye hello too
```

目標檔案：

```text
user/xargs.c
```

並且加入 `Makefile`：

```makefile
$U/_xargs\
```

### 我的解法

```c
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/param.h"

int
main(int argc, char* argv[])
{
  if(argc < 2){
    fprintf(2, "Usage: xargs <command>\n");
    exit(1);
  }

  char *xargv[MAXARG];
  int xargc = 0;

  for(int i = 1; i < argc; i++){
    xargv[xargc++] = argv[i];
  }

  int base_argc = xargc;
  char buf[512];
  int buf_idx = 0;
  char c;
  int in_word = 0;
  char *word_start = buf;

  while(read(0, &c, 1) > 0){
    if(c == ' ' || c == '\n'){
      buf[buf_idx++] = '\0';

      if(in_word){
        xargv[xargc++] = word_start;
        in_word = 0;
      }

      if(c == '\n'){
        xargv[xargc] = 0;

        if(fork() == 0){
          exec(xargv[0], xargv);
          fprintf(2, "xargs: exec %s failed\n", xargv[0]);
          exit(1);
        } else {
          wait(0);
        }

        xargc = base_argc;
        buf_idx = 0;
      }
    } else {
      if(!in_word){
        word_start = &buf[buf_idx];
        in_word = 1;
      }

      buf[buf_idx++] = c;
    }
  }

  exit(0);
}
```

### 解法說明

這份解法分成三個部分：

1. 先把 `xargs` 後面的 command 和固定參數存進 `xargv`。
2. 從 stdin 一次讀一個字元，用空白和換行切出參數。
3. 每讀完一行，就 fork 子 process，透過 `exec(xargv[0], xargv)` 執行 command。

以這個例子來看：

```bash
echo hello too | xargs echo bye
```

初始參數是：

```text
xargv[0] = "echo"
xargv[1] = "bye"
```

stdin 讀到 `hello too\n` 後，會把 `hello` 和 `too` 附加進 `xargv`：

```text
echo bye hello too
```

這題的重點是理解 `buf` 和 `xargv` 的關係：`buf` 存真正的字元內容，`xargv` 存的是指向 `buf` 裡不同位置的指標。遇到空白或換行時，把該位置改成 `\0`，就能把同一個 buffer 切成多個 C 字串。

測試：

```bash
sh < xargstest.sh
```

## 小結

LAB1 主要是在熟悉 xv6 user space 的基本工具與 system call 使用方式：

- `sleep`：使用 system call。
- `pingpong`：使用 `pipe` 和 `fork` 做 process 間通訊。
- `primes`：用多個 process 串成 pipe pipeline。
- `find`：練習讀目錄與遞迴。
- `xargs`：練習 stdin parsing、fork、exec、wait。

這些題目看起來都是小工具，但其實會一路碰到後面 OS 會用到的核心概念：process、file descriptor、pipe、trap、system call 和 kernel / user space 的邊界。
