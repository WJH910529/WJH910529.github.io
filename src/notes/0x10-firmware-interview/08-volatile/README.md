---
title: "[0x10-08] volatile：外部可變狀態與正確限制"
date: 2026-07-14
category: [Notes]
tag: [firmware, embedded-c, interview, volatile]
---

# [0x10-08] `volatile`：外部可變狀態與正確限制

## 原始來源連結

- [A 'C' Test — Volatile, Question 8](https://rmbconsulting.us/publications/a-c-test-the-0x10-best-questions-for-would-be-embedded-programmers/#volatile)

## 題目

`volatile` 關鍵字代表什麼？請舉出三種用途，並回答：

1. 一個物件能否同時是 `const` 與 `volatile`？
2. 指標本身能否是 `volatile`？
3. 下列函式有什麼問題？

```c
int square(volatile int *ptr)
{
    return *ptr * *ptr;
}
```

## 先說結論／面試回答

`volatile` 表示物件可能被目前程式碼看不到的因素改變，因此每個 volatile access 都必須依 C 實作對 volatile 的規則實際處理；compiler 不能只憑一般資料流分析就把值永久留在暫存器，或合併、刪除必要存取。

嵌入式常見例子有：

1. memory-mapped peripheral 的狀態或控制暫存器；
2. 主程式與 ISR 之間使用、且可能被 ISR 非同步修改的旗標；
3. 可能由 DMA、另一個硬體執行單元，或特定 RTOS 執行環境改變的物件。

原文也把「多個 task 共用的變數」列為例子；以現代 C memory model 來看，**只加 `volatile` 不足以讓 task 間共享資料正確**，仍需 atomic、mutex、critical section 或 RTOS 同步原語。

補充題答案：

- 可以同時是 `const volatile`。例如唯讀狀態暫存器：程式不應寫它，所以是 `const`；硬體會更新它，所以是 `volatile`。
- 指標本身可以是 volatile，例如 `int * volatile current_buffer;` 表示指標值可能被非同步改變。這和 `volatile int *p`（指向 volatile int）修飾不同層。
- `*ptr * *ptr` 會讀取 volatile 物件兩次；兩次之間值可能改變，結果可能是兩個不同值相乘，不一定是某個值的平方。正確做法是先讀取一次快照：

```c
long long square(volatile int *ptr)
{
    int snapshot = *ptr;
    return (long long)snapshot * snapshot;
}
```

這裡同時把第一個 operand 轉成 `long long`，讓乘法不會先以較窄的 `int` 進行；這是另一個與 `volatile` 分開考量的整數範圍問題。

## 觀念拆解

### `volatile` 約束存取，不是防止變更

`const` 限制程式透過某個 lvalue 寫入；`volatile` 則提醒 compiler，該物件的 access 具有不可隨意省略的外部意義。它不會阻止數值改變，反而就是用來描述「數值可能在非預期時間改變」。

對 MMIO 而言，讀取可能清除狀態，寫入可能啟動硬體，多一次或少一次 access 都可能改變裝置行為。不過 C 標準本身不定義 peripheral bus transaction；最終仍要依 compiler 的 volatile 語意、目標架構與裝置文件判斷。

### `const volatile` 與 volatile pointer

```c
const volatile unsigned int *status;
```

`status` 是一般指標，指向程式只能讀、但外部可能改變的整數。

```c
unsigned char * volatile active_buffer;
```

`active_buffer` 指向一般資料，但指標值本身可能被外部流程改變。若兩層都可能改變，可寫成 `volatile unsigned char * volatile p`。

### 平方函式為什麼要 snapshot

對 `volatile int *ptr` 而言，原式明確出現兩次 `*ptr`，所以必須接受兩次觀察可能不同。若第一次讀到 3，硬體接著改成 4，第二次讀到 4，結果是 12，不是 9 或 16。先存成一般區域變數，則只做一次 volatile read，再對穩定快照計算。

### `volatile` 沒有保證什麼

`volatile` **不保證**以下任何一項：

- **atomicity**：一次存取可能需要多條指令，仍可能被中斷或觀察到中間狀態；
- **thread safety**：C11 中，多執行緒對非 atomic 物件發生衝突存取而沒有同步，仍是 data race 與 undefined behavior；
- **memory ordering**：它不建立 inter-thread happens-before，也不是通用 CPU memory barrier 或 compiler barrier；
- **cache coherency**：DMA 與 CPU cache 不一致時，通常還要做 cache clean/invalidate 或使用 coherent memory；
- **複合操作不可分割**：`counter++` 仍是 read-modify-write，不會因 volatile 而成為單一不可分割操作。

task 間同步應使用 C11 `<stdatomic.h>`（若工具鏈與平台支援）、RTOS mutex/semaphore、critical section 或架構專用同步原語；MMIO ordering 則依晶片與 compiler 提供的 barrier API 處理。

## 自我實作

桌面環境沒有真實 peripheral 與 ISR，因此實驗分兩部分：用標準 C signal handler 示範 handler 與主流程透過合法旗標溝通，再用函式明確模擬「硬體在兩次讀取間更新暫存器」。`raise()` 是受控觸發，而且會等 handler 返回後才返回，不能把這次執行視為真正的任意時間 interleaving。完整程式存放在同目錄的 `example.c`。

```c
#include <signal.h>
#include <stdio.h>

static volatile sig_atomic_t event_flag;
static volatile int simulated_status = 3;

static void handle_signal(int signal_number)
{
    (void)signal_number;
    event_flag = 1;
}

static void simulate_hardware_update(void)
{
    ++simulated_status;
}

int main(void)
{
    int first_read;
    int second_read;
    int snapshot;

    if (signal(SIGINT, handle_signal) == SIG_ERR) {
        fputs("failed to install signal handler\n", stderr);
        return 1;
    }

    printf("event flag before signal: %d\n", (int)event_flag);
    if (raise(SIGINT) != 0) {
        fputs("failed to raise signal\n", stderr);
        return 1;
    }
    printf("event flag after signal : %d\n", (int)event_flag);

    first_read = simulated_status;
    simulate_hardware_update();
    second_read = simulated_status;
    printf("two reads: %d then %d, product = %d\n",
           first_read,
           second_read,
           first_read * second_read);

    simulated_status = 3;
    snapshot = simulated_status;
    simulate_hardware_update();
    printf("one snapshot: %d, square = %d, register is now %d\n",
           snapshot,
           snapshot * snapshot,
           simulated_status);

    return 0;
}
```

`volatile sig_atomic_t` 是 C 標準特別允許 signal handler 寫入、一般流程讀取的旗標型態；這只示範單一旗標，不代表任意 volatile 型別都有相同的 signal/ISR atomicity。

## 編譯與執行指令

本次實測環境是 Windows 上的 **32-bit MinGW GCC 6.3.0 編譯目標**（`gcc -dumpmachine` 為 `mingw32`，`int` 與 pointer 都是 4 bytes）。在本篇目錄執行：

```powershell
gcc -std=c11 -Wall -Wextra -pedantic example.c -o example.exe
.\example.exe
```

編譯時沒有 warning 或 error。

## 本機實際跑出的輸出

```text
event flag before signal: 0
event flag after signal : 1
two reads: 3 then 4, product = 12
one snapshot: 3, square = 9, register is now 4
```

## 結果理解

- `event_flag` 先是 0；`raise(SIGINT)` 受控地執行 handler 後變成 1，示範 signal handler 與主流程能以 `volatile sig_atomic_t` 旗標溝通，但沒有重現真正的非同步 interleaving。
- 模擬暫存器第一次讀到 3，接著明確模擬硬體更新，再讀時已是 4；相乘得到 12，說明原題 `*ptr * *ptr` 的風險。
- 第二輪只讀一次並保存 `snapshot = 3`。即使模擬暫存器隨後變成 4，快照平方仍穩定得到 9。
- 桌面測試以一般函式模擬硬體更新，驗證的是多次讀取與單次快照的語意差異，不是宣稱 Windows signal 等同 MCU ISR 或真實 MMIO。

## 嵌入式／平台注意事項

- ISR 與主程式共用資料時，要確認 CPU 能否以單一不可分割指令存取該寬度；即使單次讀寫是 atomic，複合 read-modify-write 仍可能需要 critical section。
- peripheral register 常由供應商 header 宣告成 `volatile` 或 `const volatile`。應沿用裝置定義的寬度與 access type，不要自行假設 `int` 大小。
- read-to-clear、write-one-to-clear 或有時序要求的暫存器，多做一次 debug read 都可能造成副作用。
- volatile access 與一般 RAM access 之間若需要順序保證，使用平台文件指定的 memory barrier，不要把 `volatile` 當成可攜 barrier。
- DMA buffer 若經 data cache，通常還需要 cache maintenance 與 ownership protocol。
- RTOS task 共享資料使用 RTOS 保證的同步 API，不要因輪詢測試「看起來會動」就把 volatile flag 當成完整同步設計。

## 小結

`volatile` 的核心不是「變數不會被最佳化」，而是 compiler 必須尊重 volatile access 可能連接到外部狀態。它適合 MMIO、ISR/signal 旗標等場景，但不提供 atomicity、thread safety 或 memory ordering。需要一致快照時只讀一次；需要同步時使用真正的 atomic、鎖、critical section 與 barrier。

## 參考資料

- [RMB Consulting — A 'C' Test](https://rmbconsulting.us/publications/a-c-test-the-0x10-best-questions-for-would-be-embedded-programmers/#volatile)
- [ISO/IEC 9899:201x Committee Draft N1570（5.1.2.3、6.7.3、7.14）](https://www.open-std.org/jtc1/sc22/wg14/www/docs/n1570.pdf)
- [GCC Manual — When is a Volatile Object Accessed?](https://gcc.gnu.org/onlinedocs/gcc/Volatiles.html)
- [0x10 題繁中整理](https://hackmd.io/GqAzU2LpRbqapcnNYUpEJw)
- [Medium 系列總覽：工程師應知道的 0x10 個問題](https://medium.com/@racktar7743/%E5%B7%A5%E7%A8%8B%E5%B8%AB%E6%87%89%E7%9F%A5%E9%81%93%E7%9A%840x10%E5%80%8B%E5%95%8F%E9%A1%8C-%E7%B8%BD%E8%A6%BD-970cc31f6b76)
