---
title: "[0x10-13] 字長與補數：避免 0xFFFF 的平台假設"
date: 2026-07-14
category: [Notes]
tag: [firmware, embedded-c, interview, integer-width]
---

# [0x10-13] 字長與補數：避免 0xFFFF 的平台假設

## 原始來源

- [A 'C' Test: The 0x10 Best Questions for Would-be Embedded Programmers — Code Examples, Question 13](https://rmbconsulting.us/publications/a-c-test-the-0x10-best-questions-for-would-be-embedded-programmers/#code-examples)

## 題目

請評論下面的程式：

```c
unsigned int zero = 0;
unsigned int compzero = 0xFFFF; /* 0 的 one's complement */
```

程式想用 `0xFFFF` 表示「每一個 bit 都是 1」，但這個寫法把 `unsigned int` 的寬度偷偷假設成 16 bits。

## 先說結論／面試回答

`0xFFFF` 的數值固定是 `65535`，只在 `unsigned int` 恰好為 16 bits 時等於全 1。若 `unsigned int` 是 32 bits，真正的全 1 是 `0xFFFFFFFF`；若型別更寬，差距還會更大。

比硬編碼字長更可攜的寫法是：

```c
unsigned int compzero = ~0u;
```

或直接使用標準標頭中的界限：

```c
unsigned int compzero = UINT_MAX;
```

`u` 很重要：`~0u` 的 operand 從一開始就是 `unsigned int`，逐 bit 反相後明確得到該型別的最大值。原文提出的 `~0` 在本機二補數平台會先產生 `int` 的 `-1`，再轉成 `unsigned int`；雖然本機結果相同，`~0u` 更直接表達型別與意圖。

## 觀念拆解

### 常數的「數值」不等於儲存型別的「位元圖樣」

`0xFFFF` 是一個整數常數，其數值就是十進位 `65535`。指定給 32-bit `unsigned int` 時會成為：

```text
00000000 00000000 11111111 11111111
```

只有低 16 bits 是 1，並不是 32 bits 全部為 1。程式碼看起來像 bit mask，實際上卻綁死某個 word length。

### `~` 是逐位元反相

對 unsigned operand 而言，`~` 把型別中每個 value bit 翻轉。`0u` 的所有 bits 都是 0，因此 `~0u` 會得到 `UINT_MAX`。若已經有 unsigned 變數 `zero`，寫 `~zero` 也具有相同型別與效果。

### 固定寬度需求要明說

如果規格真的要求 16-bit mask，應使用 `uint16_t`（平台有提供時）並搭配 `UINT16_MAX`；如果要求「某個 `unsigned int` 的所有 bits」，就使用 `~0u` 或 `UINT_MAX`。這兩種需求不能都用 `0xFFFF` 含糊帶過。

## 自我實作

範例同時輸出型別寬度、硬編碼值與逐 bit 反相值。完整程式也放在同目錄的 `example.c`：

```c
#include <limits.h>
#include <stdio.h>

int main(void)
{
    unsigned int zero = 0u;
    unsigned int hard_coded = 0xFFFFu;
    unsigned int compzero = ~zero;

    printf("unsigned int object bits = %u\n",
           (unsigned int)(sizeof(unsigned int) * CHAR_BIT));
    printf("UINT_MAX = %u\n", UINT_MAX);
    printf("0xFFFF = %u (0x%08X)\n", hard_coded, hard_coded);
    printf("~zero = %u (0x%08X)\n", compzero, compzero);
    puts(hard_coded == compzero ? "same value? yes" : "same value? no");

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
unsigned int object bits = 32
UINT_MAX = 4294967295
0xFFFF = 65535 (0x0000FFFF)
~zero = 4294967295 (0xFFFFFFFF)
same value? no
```

編譯器沒有產生 warning。

## 結果理解

`sizeof(unsigned int) * CHAR_BIT` 顯示本機 object representation 是 32 bits；理論上這個數字可能包含 padding bits，但 `UINT_MAX = 4294967295` 也證明本機確實有 32 個 value bits。輸出的十六進位格式最直觀：`0x0000FFFF` 只設定低 16 bits，而 `0xFFFFFFFF` 才是 32-bit 全 1。兩者比較結果為 `no`，直接驗證原始註解在這個目標平台不成立。

這也說明為什麼嵌入式工程師要對資料寬度敏感：原始碼能成功編譯，不代表 mask 涵蓋了預期的硬體欄位。

## 嵌入式／平台注意事項

- C 只保證 `unsigned int` 至少 16 bits，不保證固定為 16 或 32 bits；使用前查看 `<limits.h>` 與 compiler ABI。
- 操作硬體暫存器時，優先採用晶片 header 提供的暫存器型別與 mask，並確認 register 寬度。
- `uint16_t`、`uint32_t` 是「若平台存在就恰好該寬度」的型別；對應最大值可用 `UINT16_MAX`、`UINT32_MAX`。
- 運算較窄型別時仍會發生 integer promotions。必要時在最後明確 cast 回目標寬度，並檢查中間運算型別。
- 輸出時格式也要匹配型別；固定寬度整數可使用 `<inttypes.h>` 的 `PRIu32`、`PRIX32` 等巨集。

## 小結

`0xFFFF` 表示的是數值 `65535`，不是抽象的「所有 bits 都是 1」。若目標是 `unsigned int` 的全 1，使用 `~0u` 或 `UINT_MAX`；若目標是固定 16 bits，則用明確的固定寬度型別與常數。

## 參考資料

- [RMB Consulting：原始第 13 題](https://rmbconsulting.us/publications/a-c-test-the-0x10-best-questions-for-would-be-embedded-programmers/#code-examples)
- [ISO C11 working draft N1570，6.2.6、6.4.4.1、6.5.3.3](https://www.open-std.org/jtc1/sc22/wg14/www/docs/n1570.pdf)
