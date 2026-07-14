---
title: "[0x10-04] 無窮迴圈：在 C 中撰寫 Infinite Loop"
date: 2026-07-14
category: [Notes]
tag: [firmware, embedded-c, interview, control-flow]
---

# [0x10-04] 無窮迴圈：在 C 中撰寫 Infinite Loop

## 原始來源

- [A 'C' Test: The 0x10 Best Questions for Would-be Embedded Programmers — Infinite Loops](https://rmbconsulting.us/publications/a-c-test-the-0x10-best-questions-for-would-be-embedded-programmers/#infinite-loops)

## 題目

嵌入式系統經常需要無窮迴圈。如何用 C 寫出一個無窮迴圈？

最常看到的兩種答案是：

```c
while (1) {
    /* main loop */
}
```

以及：

```c
for (;;) {
    /* main loop */
}
```

## 先說結論／面試回答

我會回答：`while (1)` 和 `for (;;)` 在 C 中都能表達刻意不結束的迴圈，實務上應遵守專案的 coding style。`for (;;)` 的三個欄位全部省略，條件自然永遠成立；`while (1)` 則直接用永遠為真的整數常數表達意圖。

真正重要的不是背出哪一種拼法，而是知道韌體主迴圈不能只做無意義的 busy-wait。它通常還會輪詢事件、驅動狀態機、餵 watchdog，或在沒有工作時執行 sleep／wait-for-interrupt，並且必須分析 CPU 使用率、功耗與系統是否仍能回應中斷。

## 觀念拆解

### `for (;;)` 為什麼不需要條件

`for` 的完整形式是：

```c
for (初始化; 條件; 更新) {
    /* body */
}
```

三個部分都可以省略。當中間的控制運算式被省略時，它等同於永遠為真，所以 `for (;;)` 不會自行結束；只有 `break`、`return`、`goto`、例外的控制流程或外部終止能離開它。

### `while (1)` 會不會每圈都重新判斷

在 C 語意上，每次迭代都會評估條件 `1`，結果永遠為真。一般最佳化編譯器可以直接辨識這是常數條件，通常不會真的產生「每圈比較 1」的額外指令。不過最後機器碼仍應以實際編譯器、最佳化等級與目標 MCU 為準。

若想寫成 `while (true)`，C 程式需要包含 `<stdbool.h>`；`true` 會展開成布林真值。`while (1)` 不需要額外標頭，因此在舊式或很小的 C 專案中仍很常見。

### 無窮迴圈不等於卡死

有設計目的的 super loop 會持續推動整個系統，例如：

```text
讀取事件 -> 更新狀態機 -> 執行到期工作 -> 進入低功耗等待 -> 醒來後重複
```

相反地，如果程式因為錯誤條件永遠出不去，阻斷其他必要工作，那才是 bug。面試時可以主動區分「設計上的主迴圈」與「程式意外卡死」。

## 自我實作

真的執行無窮迴圈會讓自動測試永遠不結束，所以我的範例使用 `for (;;)` 寫主迴圈，但預設在第 3 圈用 `break` 停止。完整程式放在同目錄的 `example.c`：

```c
#include <stdio.h>

#ifndef DEMO_LIMIT
#define DEMO_LIMIT 3U
#endif

int main(void)
{
    unsigned int iteration = 0U;

    puts("enter main loop");
    for (;;) {
        ++iteration;
        printf("iteration %u: poll devices and handle events\n", iteration);

#if DEMO_LIMIT > 0U
        if (iteration >= DEMO_LIMIT) {
            puts("demo only: break so the host test can finish");
            break;
        }
#endif
    }

    printf("loop body ran %u times\n", iteration);
    return 0;
}
```

`DEMO_LIMIT` 只是桌面測試的安全開關。若編譯時定義成 `0`，預處理器會移除整段停止條件，留下真正的無窮迴圈。

## 編譯與執行指令

測試環境是 Windows 上的 **32-bit MinGW GCC 6.3.0 編譯目標**（`gcc -dumpmachine` 為 `mingw32`）：

```powershell
gcc -std=c11 -Wall -Wextra -pedantic example.c -o example.exe
.\example.exe
```

## 本機實際輸出

```text
enter main loop
iteration 1: poll devices and handle events
iteration 2: poll devices and handle events
iteration 3: poll devices and handle events
demo only: break so the host test can finish
loop body ran 3 times
```

編譯器沒有產生 warning。

## 結果理解

輸出證明 `for (;;)` 會持續回到迴圈頂端；三次內容完全相同，只是計數器增加。程式之所以能走到最後一行，是因為示範版明確執行了 `break`，不是迴圈自己結束。

真正的無窮版本可以這樣編譯：

```powershell
gcc -std=c11 -Wall -Wextra -pedantic -DDEMO_LIMIT=0 example.c -o forever.exe
```

我沒有在自動測試中執行 `forever.exe`，因為它依設計不會返回；這不是未驗證，而是避免把測試程序永久占住。

## 嵌入式／平台注意事項

- Bare-metal 韌體離開 `main()` 後要做什麼由啟動碼與平台決定，因此通常會刻意保留主迴圈；不要假設它會像桌面程式一樣正常回到作業系統。
- 純 busy loop 會持續耗電。若硬體允許，空閒時可使用 WFI、sleep mode 或 RTOS idle task。
- 每圈工作都必須有可預測的最壞執行時間，否則低優先工作可能永遠得不到服務。
- Watchdog 不能只在固定位置無條件餵食；更好的設計是確認關鍵工作仍健康後才餵，否則系統卡住時 watchdog 也救不了它。
- 若共享狀態由 ISR 修改，還要另外處理 `volatile`、原子性與競爭條件；無窮迴圈本身不提供同步保證。

## 小結

`while (1)` 與 `for (;;)` 都是合法且常見的答案。完整的韌體觀念則是：主迴圈為什麼永遠存在、每圈做什麼、沒有工作時如何省電，以及如何保證中斷、排程與 watchdog 仍能正常運作。

## 參考資料

- [RMB Consulting：原始第 4 題](https://rmbconsulting.us/publications/a-c-test-the-0x10-best-questions-for-would-be-embedded-programmers/#infinite-loops)
- [Medium：工程師應知道的 0x10 個問題（4）— 無窮迴圈](https://medium.com/@racktar7743/%E5%B7%A5%E7%A8%8B%E5%B8%AB%E6%87%89%E7%9F%A5%E9%81%93%E7%9A%840x10%E5%80%8B%E5%95%8F%E9%A1%8C-4-%E7%84%A1%E7%AA%AE%E8%BF%B4%E5%9C%88-47a498122c55)
- [ISO C11 working draft N1570](https://www.open-std.org/jtc1/sc22/wg14/www/docs/n1570.pdf)
