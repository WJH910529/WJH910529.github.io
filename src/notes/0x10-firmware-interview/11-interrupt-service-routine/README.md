---
title: "[0x10-11] 中斷：檢視 Interrupt Service Routine"
date: 2026-07-14
category: [Notes]
tag: [firmware, embedded-c, interview, interrupt]
---

# [0x10-11] 中斷：檢視 Interrupt Service Routine

## 原始來源

- [A 'C' Test: The 0x10 Best Questions for Would-be Embedded Programmers — Interrupts](https://rmbconsulting.us/publications/a-c-test-the-0x10-best-questions-for-would-be-embedded-programmers/#interrupts)

## 題目

原題用編譯器額外提供的 `__interrupt` 關鍵字宣告下列中斷服務常式（Interrupt Service Routine, ISR），要求指出程式中的問題：

```c
__interrupt double compute_area(double radius)
{
    double area = PI * radius * radius;
    printf("\nArea = %f", area);
    return area;
}
```

## 先說結論／面試回答

這段程式把 ISR 當成一般函式使用，介面與工作內容都不恰當：

1. `__interrupt` 並非標準 C，而是編譯器／MCU 的擴充；ISR 的函式原型、向量表連接方式及暫存器保存規則都要遵守該平台 ABI。
2. 原題中的 ISR 接收一般函式參數並回傳 `double`。硬體中斷不是由 C 呼叫端依一般 calling convention 呼叫，因此這種介面通常不符合 ISR ABI；實際允許的 signature 仍以 vendor 文件為準。
3. 浮點運算可能需要額外保存 FPU 暫存器、呼叫軟體浮點函式，甚至根本不能安全地在該 ISR 環境使用，會拉長中斷延遲。
4. `printf` 通常很慢，還可能使用鎖、heap、全域緩衝區或不可重入的底層 I/O。若它等待 UART，ISR 甚至可能卡住整個系統。
5. ISR 應只確認／清除中斷來源、擷取必要資料並通知主迴圈或工作執行緒；面積計算與輸出應延後處理。

一個合適的面試回答不只要說「ISR 不應回傳值」，還要進一步談 ABI、可重入性、最壞執行時間與 deferred work。

## 觀念拆解

### ISR 不是普通函式

一般函式由另一段程式依 calling convention 呼叫；硬體 ISR 則由中斷向量進入。進入與離開時需要保存哪些狀態、用哪一條特殊 return-from-interrupt 指令，以及能否巢狀中斷，都是 MCU 與編譯器 ABI 的責任。不同平台可能使用 `__interrupt`、函式 attribute、pragma、專用函式名稱或向量表巨集，不能把某一家的語法當成標準 C。

### 為什麼要讓 ISR 短小

執行 ISR 時，等優先級或較低優先級的工作往往無法執行。ISR 越長，中斷延遲與系統 jitter 越難預測。浮點、格式化輸出、動態配置及可能阻塞的函式，都不適合未經分析就放進 ISR。

### ISR 與主程式如何溝通

常見做法是 ISR 設定旗標、寫入 lock-free ring buffer、釋放 RTOS 的 ISR-safe semaphore，或送出事件；主迴圈／工作執行緒再完成耗時工作。共享資料還要考慮原子性、可見性與「檢查後清除」之間是否會漏掉新事件。`volatile` 只能限制編譯器最佳化，不等於跨核心同步工具，也不自動讓多位元組存取成為原子操作。

## 自我實作

這裡無法用標準 C 製造真正的 MCU 硬體中斷，因此把 `simulated_timer_isr()` 當成 ISR 入口，刻意限制它只設定旗標。面積計算與 `printf` 都留在 `main` 的 deferred work。完整程式也放在同目錄的 `example.c`：

```c
#include <signal.h>
#include <stdio.h>

#define PI 3.141592653589793

static volatile sig_atomic_t work_pending = 0;
static double requested_radius = 0.0;

static void simulated_timer_isr(void)
{
    work_pending = 1;
}

static void process_deferred_work(void)
{
    double area = PI * requested_radius * requested_radius;

    printf("main: deferred area for radius %.1f = %.3f\n",
           requested_radius, area);
}

int main(void)
{
    requested_radius = 3.0;
    puts("main: trigger simulated interrupt");
    simulated_timer_isr();

    printf("main: pending flag = %d\n", (int)work_pending);
    if (work_pending != 0) {
        work_pending = 0;
        process_deferred_work();
    }
    printf("main: pending flag after work = %d\n", (int)work_pending);

    return 0;
}
```

`sig_atomic_t` 讓這個桌面版範例清楚表達「非同步處理器只改一個旗標」的意圖；它不是對所有 MCU ISR 原子性的保證。

## 編譯與執行指令

測試環境是 Windows 上的 **32-bit MinGW GCC 6.3.0 編譯目標**（`gcc -dumpmachine` 為 `mingw32`）：

```powershell
gcc -std=c11 -Wall -Wextra -pedantic example.c -o example.exe
.\example.exe
```

## 本機實際輸出

```text
main: trigger simulated interrupt
main: pending flag = 1
main: deferred area for radius 3.0 = 28.274
main: pending flag after work = 0
```

編譯器沒有產生 warning。

## 結果理解

執行 `simulated_timer_isr()` 後只有 `work_pending` 由 `0` 變成 `1`，ISR 本身沒有輸出，也沒有做浮點計算。`main` 看到旗標後先清除它，再呼叫 `process_deferred_work()`，所以面積與文字輸出都發生在一般程式脈絡。這就是簡化版的 top half／bottom half 分工。

本例是同步呼叫的安全模擬，不會真的與 `main` 競爭。真實硬體若允許 ISR 在主程式清旗標時再次發生，應依平台使用短暫關中斷、原子交換、事件計數器或 RTOS 的 ISR-safe API，避免遺失事件。

## 嵌入式／平台注意事項

- 查閱 MCU 啟動碼、向量表與編譯器 ABI，確認正確 ISR 宣告；不要直接移植其他 vendor 的 `__interrupt` 語法。
- 某些核心有 lazy FPU context stacking，但仍須量測最壞延遲，不能因此假設 ISR 內浮點「免費」。
- ISR 呼叫的函式必須可重入、不可阻塞，並且是文件明確允許的 ISR-safe API。
- 若旗標寬度超過核心可原子存取寬度，單純加 `volatile` 仍可能讀到撕裂值。
- 中斷來源通常要在正確時機 acknowledge／clear，否則離開 ISR 後可能立即再進入。

## 小結

ISR 的重點不是加上一個特殊關鍵字，而是遵守平台 ABI 並控制工作量。最穩健的方向是：ISR 快速收事件、主迴圈或 RTOS task 做耗時工作。

## 參考資料

- [RMB Consulting：原始第 11 題](https://rmbconsulting.us/publications/a-c-test-the-0x10-best-questions-for-would-be-embedded-programmers/#interrupts)
- [GCC 6.3.0 Manual：Function Attributes](https://gcc.gnu.org/onlinedocs/gcc-6.3.0/gcc/Function-Attributes.html)
- [ISO C11 working draft N1570](https://www.open-std.org/jtc1/sc22/wg14/www/docs/n1570.pdf)
