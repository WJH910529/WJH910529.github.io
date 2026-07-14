---
title: "[0x10-14] 動態記憶體：嵌入式系統使用 Heap 的風險"
date: 2026-07-14
category: [Notes]
tag: [firmware, embedded-c, interview, dynamic-memory]
---

# [0x10-14] 動態記憶體：嵌入式系統使用 Heap 的風險

## 原始來源

- [A 'C' Test: The 0x10 Best Questions for Would-be Embedded Programmers — Dynamic Memory Allocation](https://rmbconsulting.us/publications/a-c-test-the-0x10-best-questions-for-would-be-embedded-programmers/#dynamic-memory-allocation)

## 題目

原題分成兩部分：

1. 嵌入式系統從 heap 動態配置記憶體會遇到哪些問題？
2. 下列程式會輸出哪一行？為什麼？

```c
char *ptr;

if ((ptr = (char *)malloc(0)) == NULL) {
    puts("Got a null pointer");
} else {
    puts("Got a valid pointer");
}
```

## 先說結論／面試回答

嵌入式系統使用動態記憶體的主要風險包括：

- **碎片化**：總剩餘空間可能足夠，卻找不到夠大的連續區塊。
- **執行時間不確定**：配置器搜尋、分割、合併 free block 或取得鎖的時間會依 heap 狀態改變，難以估算 WCET。
- **配置失敗**：RAM 有限，長時間運作後的失敗路徑必須被設計與測試，不能假設 `malloc` 永遠成功。
- **生命週期錯誤**：memory leak、double free、use-after-free 與錯誤 ownership 都可能造成難以重現的故障。
- **並行與可重入性**：主程式、task 與 ISR 共用 allocator 時可能需要鎖；一般 `malloc` 通常不是 ISR-safe。
- **回收停頓**：若系統使用具有 garbage collector 的執行環境，回收時間也可能破壞即時性；純 C 的 `malloc/free` 本身沒有自動 GC。

至於 `malloc(0)`，正確答案是「依實作而定」。C11 允許它回傳 null pointer，也允許回傳一個不得拿來存取物件的 non-null pointer。若回傳 non-null，仍可把原值交給 `free`。

## 觀念拆解

### 碎片化不等於記憶體完全用完

外部碎片是空閒空間被切成很多不連續的小洞；內部碎片通常是 alignment 或 size-class rounding 讓分配到的區塊大於請求大小。Allocator metadata 則是另一項管理額外負擔，也會消耗 RAM，但不必全部歸類為 internal fragmentation。長時間交錯配置不同大小、不同生命週期的物件，會讓 heap 狀態逐漸依賴執行歷史。

例如系統仍有 100 bytes 空閒，但分成 40、30、30 三塊時，要求一塊連續 64 bytes 仍可能失敗。這對必須連續運作數月或數年的裝置尤其危險。

### 即時系統關心的是上限，不只是平均值

桌面程式常在意平均配置速度；hard real-time 系統在意的是最壞情況能否在 deadline 內完成。一般用途 allocator 的步驟可能與 free-list 長度、相鄰區塊與鎖競爭有關，因此單次 `malloc` 的上限不容易證明。

### `malloc(0)` 沒有可寫的 0-byte 物件

即使拿到 non-null pointer，也不能因為「看起來有效」就解參考或寫入。這個結果只能用來比較、保留原值後傳給 `free`，或直接不讓 0-byte 請求進入 allocator。將回傳結果強制轉成 `char *` 不會改變規則；在 C 中也不需要對 `malloc` 回傳的 `void *` 做 cast。

## 自我實作

範例不輸出位址，因為位址每次執行可能不同；只記錄這個 C runtime 回傳 NULL 或 non-NULL，然後一律呼叫 `free(ptr)`。完整程式也放在同目錄的 `example.c`：

```c
#include <stdio.h>
#include <stdlib.h>

int main(void)
{
    void *ptr = malloc(0);

    if (ptr == NULL) {
        puts("malloc(0): NULL");
    } else {
        puts("malloc(0): non-NULL");
    }

    puts("calling free(ptr)");
    free(ptr);
    puts("free(ptr): completed");

    return 0;
}
```

## 編譯與執行指令

測試環境是 Windows 上的 **32-bit MinGW GCC 6.3.0 編譯目標**（`gcc -dumpmachine` 為 `mingw32`）：

```powershell
gcc -std=c11 -Wall -Wextra -pedantic example.c -o example.exe
.\example.exe
```

## 本機實際輸出

```text
malloc(0): non-NULL
calling free(ptr)
free(ptr): completed
```

編譯器沒有產生 warning。本次 MinGW／Windows C runtime 實際回傳 non-NULL；這只是本機觀察，不能外推成所有 C 實作都會如此。

## 結果理解

第一行證實這次 `malloc(0)` 得到 non-null pointer，但程式沒有讀寫它指向的位置。接著 `free(ptr)` 正常完成，符合「把 allocator 回傳的原指標交回 `free`」的用法。若另一個實作回傳 NULL，同一段 `free(ptr)` 也安全，因為 `free(NULL)` 不做任何事。

所以本題不能只背某台電腦的輸出。面試真正要觀察的是能否區分「標準允許的結果」與「當前 runtime 的實際結果」，並知道 non-null 不代表可解參考。

## 嵌入式／平台注意事項

- 安全關鍵或 hard real-time 路徑常在初始化階段一次配置完成，運行期不再呼叫一般 heap allocator。
- 若運行期確實需要配置，可考慮固定大小 memory pool、slab、arena 或具明確時間上限的 RTOS allocator。
- 針對每個配置點設計失敗策略，並用長時間壓力測試觀察 fragmentation；只測「開機後第一次配置」沒有代表性。
- 不要在 ISR 中呼叫一般 `malloc/free`，除非 vendor 明確保證 ISR-safe 且延遲符合需求。
- `realloc`、alignment、allocator metadata 與 thread safety 也都會增加 RAM 與時間成本。
- 若請求大小可能由乘法算出，先檢查 size overflow，否則可能配置到比預期小的區塊後再越界寫入。

## 小結

動態配置不是嵌入式系統的禁忌，但必須把碎片化、失敗處理與最壞延遲納入設計。`malloc(0)` 則提醒我們：可攜 C 程式不能把單一 runtime 的行為誤當成語言保證。

## 參考資料

- [RMB Consulting：原始第 14 題](https://rmbconsulting.us/publications/a-c-test-the-0x10-best-questions-for-would-be-embedded-programmers/#dynamic-memory-allocation)
- [ISO C11 working draft N1570，7.22.3 與 7.22.3.4](https://www.open-std.org/jtc1/sc22/wg14/www/docs/n1570.pdf)
