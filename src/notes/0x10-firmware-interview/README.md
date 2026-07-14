---
title: 0x10 韌體面試問題集
icon: microchip
article: false
timeline: false
---

# 0x10 韌體面試問題集

這個系列整理韌體與嵌入式 C 面試圈常見的「0x10 個問題」。`0x10` 是十六進位表示法，換成十進位就是 16，因此整套問題共有 16 題。

## 來源

- 英文原文：[A 'C' Test: The 0x10 Best Questions for Would-be Embedded Programmers](https://rmbconsulting.us/publications/a-c-test-the-0x10-best-questions-for-would-be-embedded-programmers/)
- 繁中整理：[韌體工程師應該要知道的 0x10 個問題](https://hackmd.io/GqAzU2LpRbqapcnNYUpEJw)
- 實作型文章參考：[工程師應知道的 0x10 個問題：總覽](https://medium.com/@racktar7743/%E5%B7%A5%E7%A8%8B%E5%B8%AB%E6%87%89%E7%9F%A5%E9%81%93%E7%9A%840x10%E5%80%8B%E5%95%8F%E9%A1%8C-%E7%B8%BD%E8%A6%BD-970cc31f6b76)

本系列依英文原文的題意與順序整理。繁中版本適合快速對照，但答案仍應配合目前使用的 C 標準、編譯器與硬體平台自行驗證。

## 實作與測試環境

每一題都附有可重跑的 `example.c`，而不是只列參考答案。文章中的輸出來自這個專案所在電腦的實際編譯與執行結果：

```text
作業環境：Windows
編譯器：GCC 6.3.0 (MinGW.org)
編譯目標：mingw32
C 標準：C11
sizeof(int)：4 bytes
sizeof(void *)：4 bytes
```

統一使用以下警告選項：

```bash
gcc -std=c11 -Wall -Wextra -pedantic example.c -o example.exe
.\example.exe
```

這些結果是 **32-bit MinGW C ABI** 的觀察值，不代表所有 MCU、編譯器或作業系統都會得到相同結果。涉及 ISR、memory-mapped I/O 或硬體暫存器的題目，會在一般電腦上做安全模擬，並另外說明真實嵌入式環境的差異。

## 文章列表

1. [預處理器：一年有幾秒？](01-seconds-per-year/)
2. [預處理器：撰寫安全的 `MIN` 巨集](02-min-macro/)
3. [預處理器：`#error` 指令的用途](03-error-directive/)
4. [無窮迴圈：在 C 中撰寫 Infinite Loop](04-infinite-loop/)
5. [資料宣告：讀懂指標、陣列與函式指標](05-c-declarations/)
6. [`static`：保存狀態與限制模組可見性](06-static/)
7. [`const`：唯讀語意與指標宣告](07-const/)
8. [`volatile`：外部可變狀態與正確限制](08-volatile/)
9. [位元操作：設定與清除指定 Bit](09-bit-manipulation/)
10. [固定記憶體位址：存取 Memory-Mapped 資料](10-fixed-memory-address/)
11. [中斷：檢視 Interrupt Service Routine](11-interrupt-service-routine/)
12. [整數轉型：Signed 與 Unsigned 混合運算](12-signed-unsigned/)
13. [字長與補數：避免 `0xFFFF` 的平台假設](13-word-length-complement/)
14. [動態記憶體：嵌入式系統使用 Heap 的風險](14-dynamic-memory-allocation/)
15. [型別別名：`typedef` 與 `#define` 的差異](15-typedef-vs-define/)
16. [詞法分析：`a+++b` 與 Maximum Munch](16-maximum-munch/)

## 每篇文章的整理方式

每篇文章包含以下內容：

- 題目
- 面試時可以先回答的結論
- 觀念拆解
- 可重跑的自我實作
- 實際編譯與輸出結果
- 我對結果的理解
- 嵌入式環境與平台差異
- 參考資料
