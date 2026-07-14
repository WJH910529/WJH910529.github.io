---
title: "[0x10-06] static：保存狀態與限制模組可見性"
date: 2026-07-14
category: [Notes]
tag: [firmware, embedded-c, interview, static]
---

# [0x10-06] `static`：保存狀態與限制模組可見性

## 原始來源連結

- [A 'C' Test — Static, Question 6](https://rmbconsulting.us/publications/a-c-test-the-0x10-best-questions-for-would-be-embedded-programmers/#static)

## 題目

在 C 語言中，關鍵字 `static` 有哪些用途？

## 先說結論／面試回答

原題期待的完整回答有三點：

1. **函式內的區域變數加上 `static`**：名稱仍只有該區塊看得到，但物件具有 static storage duration，只初始化一次，之後每次呼叫都保留上次的值。
2. **檔案範圍的變數加上 `static`**：物件具有 static storage duration，同時名稱具有 internal linkage；同一個 translation unit 內可用，其他 translation unit 不能直接以該名稱存取。
3. **檔案範圍的函式加上 `static`**：函式名稱具有 internal linkage，只能由同一個 translation unit 內的程式碼直接呼叫。

具有 static storage duration 的物件若沒有明確初始化，會在程式啟動前先被初始化為 0。原文使用「module」描述單一原始檔；以 C 標準用語來說，更精確的名稱是 **translation unit**，也就是原始檔經前處理後形成的編譯單位。

## 觀念拆解

`static` 會依宣告位置影響不同性質：

| 宣告位置 | Scope | Storage duration | Linkage |
| --- | --- | --- | --- |
| 函式內 `static int count;` | block scope | 整個程式執行期間 | no linkage |
| 檔案範圍 `static int total;` | file scope | 整個程式執行期間 | internal linkage |
| 檔案範圍 `static void helper(void);` | file scope | 函式不是物件，不談 storage duration | internal linkage |

一般區域變數通常具有 automatic storage duration：進入區塊時建立，離開時生命週期結束。區域 `static` 的名稱雖仍被限制在區塊內，物件本身卻一路存在到程式結束，適合保存呼叫次數、狀態機狀態或快取。

檔案範圍的 `static` 則是封裝工具。把只供單一驅動程式使用的資料與 helper function 宣告成 `static`，可以縮小可見範圍、避免其他 `.c` 檔誤用，也降低不同模組的同名符號衝突。

現代 C 另允許在 array parameter 寫 `void process(size_t n, int a[static 4]);`，表示呼叫者應提供至少 4 個元素。這不是原題年代強調的三種答案，也不會讓參數陣列變成靜態物件。

## 自我實作

以下程式同時示範檔案範圍的靜態資料、靜態函式，以及會跨呼叫保留內容的區域靜態變數。完整程式存放在同目錄的 `example.c`。

```c
#include <stdio.h>

/* File scope + static: this name has internal linkage. */
static int module_total;

/* A static function can only be named from this translation unit. */
static void add_to_total(int value)
{
    /* Block scope + static: initialized once and retained between calls. */
    static unsigned int call_count;

    ++call_count;
    module_total += value;

    printf("call %u: add %d, module_total = %d\n",
           call_count,
           value,
           module_total);
}

int main(void)
{
    printf("module_total before: %d\n", module_total);

    add_to_total(5);
    add_to_total(-2);
    add_to_total(10);

    printf("module_total after : %d\n", module_total);
    return 0;
}
```

## 編譯與執行指令

本次實測環境是 Windows 上的 **32-bit MinGW GCC 6.3.0 編譯目標**（`gcc -dumpmachine` 為 `mingw32`，`int` 與 pointer 都是 4 bytes）。在本篇目錄執行：

```powershell
gcc -std=c11 -Wall -Wextra -pedantic example.c -o example.exe
.\example.exe
```

編譯時沒有 warning 或 error。

## 本機實際跑出的輸出

```text
module_total before: 0
call 1: add 5, module_total = 5
call 2: add -2, module_total = 3
call 3: add 10, module_total = 13
module_total after : 13
```

## 結果理解

- `module_total` 沒有 initializer，仍在 `main()` 前被初始化為 0。
- `call_count` 只在 `add_to_total()` 內可見，但三次呼叫印出 1、2、3，證明同一物件保留了前一次呼叫後的內容。
- `module_total` 一路累積成 13。它與 `add_to_total()` 的名稱都因檔案範圍的 `static` 而具有 internal linkage。
- 單一檔案的輸出無法直接「印出」internal linkage；它是編譯與連結規則。若另一個 `.c` 檔只寫 `extern int module_total;` 並嘗試使用，連結器不會把這個內部符號提供給它。

## 嵌入式／平台注意事項

- 未初始化靜態物件通常放在 `.bss`，由 startup code 在 `main()` 前清為 0；有初值者通常涉及 `.data` 從 Flash 複製到 RAM。自製 linker script 或 startup code 時必須正確處理。
- 區域 `static` 不在一般函式 stack frame 中，可降低 stack 使用量，但會永久占用 RAM；若函式會讀寫這份共享的 mutable static 狀態，通常也就不再自然可重入。單純讀取 `static const` 並不會因為有 `static` 就自動破壞 reentrancy。
- `static` **不提供 atomicity 或同步**。task 與 ISR 同時讀寫同一物件時，仍要使用 critical section、atomic operation 或其他同步機制。
- header 內定義一般 `static` 函式或大型物件，可能讓每個包含它的 translation unit 各產生一份副本；小型 `static inline` helper 則是常見的刻意用法。
- C 的區域靜態物件 initializer 要符合常數初始化規則，不要套用 C++「第一次執行到宣告才動態初始化」的理解。

## 小結

回答 `static` 時不能只說「會記住上次的值」。函式內的 `static` 改變物件生命週期；檔案範圍的 `static` 則透過 internal linkage 隱藏資料或函式名稱。兩者都在幫助狀態與可見範圍符合模組設計。

## 參考資料

- [RMB Consulting — A 'C' Test](https://rmbconsulting.us/publications/a-c-test-the-0x10-best-questions-for-would-be-embedded-programmers/#static)
- [ISO/IEC 9899:201x Committee Draft N1570（6.2.2、6.2.4、6.7.1）](https://www.open-std.org/jtc1/sc22/wg14/www/docs/n1570.pdf)
- [0x10 題繁中整理](https://hackmd.io/GqAzU2LpRbqapcnNYUpEJw)
- [Medium 系列總覽：工程師應知道的 0x10 個問題](https://medium.com/@racktar7743/%E5%B7%A5%E7%A8%8B%E5%B8%AB%E6%87%89%E7%9F%A5%E9%81%93%E7%9A%840x10%E5%80%8B%E5%95%8F%E9%A1%8C-%E7%B8%BD%E8%A6%BD-970cc31f6b76)
