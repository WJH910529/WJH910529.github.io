---
title: "[0x10-02] 預處理器：撰寫安全的 MIN 巨集"
date: 2026-07-14
category: [Notes]
tag: [firmware, embedded-c, interview, macro]
---

# [0x10-02] 預處理器：撰寫安全的 `MIN` 巨集

## 原始來源

- [A 'C' Test: The 0x10 Best Questions for Would-be Embedded Programmers — Preprocessor, Question 2](https://rmbconsulting.us/publications/a-c-test-the-0x10-best-questions-for-would-be-embedded-programmers/#preprocessor)

## 題目

使用 `#define` 撰寫標準的 `MIN` 巨集：它接收兩個參數，並回傳其中較小的值。

## 先說結論／面試回答

原題的答案是：

```c
#define MIN(A, B) ((A) <= (B) ? (A) : (B))
```

我的面試回答會補上三點：

1. `A`、`B` 每次出現都加括號，整個展開結果也加括號，避免呼叫端運算子的優先順序改變語意。
2. 使用條件運算子 `?:` 選擇較小值；本題依原文使用 `<=`。
3. 這個 macro **不保證每個 argument 只求值一次**。條件先求值一次，被選中的 argument 又會求值一次，因此不能安全傳入 `i++`、函式呼叫或 volatile register read 等帶副作用的 expression。

所以這是語法正確的傳統 `MIN` macro，卻不是對任意 expression 都安全的函式替代品。若型別固定，`static inline` 函式通常更容易維護。

## 觀念拆解

### 為什麼需要這麼多括號

假設把 macro 寫成：

```c
#define BAD_MIN(A, B) A <= B ? A : B
```

呼叫 `2 * BAD_MIN(x, y)` 時，展開後的運算子結合方式可能不再是「先求最小值再乘 2」。參數本身也可能是 `x + 1` 這類 expression，因此參數與整體結果都必須用括號保護。

### Macro 是文字替換，不是函式呼叫

下面的呼叫：

```c
MIN(value++, limit)
```

展開後近似：

```c
((value++) <= (limit) ? (value++) : (limit))
```

如果條件為真，`value++` 會執行兩次。條件運算子的第一個 operand 與被選中的 operand 之間有 sequencing，因此本例不是 unsequenced modification 的未定義行為；但結果仍很可能不是開發者原先想要的值。

### `static inline` 的取捨

函式 argument 在進入函式前只求值一次，型別檢查也比較完整：

```c
static inline int min_int(int a, int b)
{
    return a <= b ? a : b;
}
```

代價是這個版本只接受可轉為 `int` 的值。傳統 macro 能套用多種相容型別，但也可能讓 signed／unsigned 轉換、不同型別比較與重複求值問題藏在展開結果裡。若需要多型介面，可依專案標準考慮 C11 `_Generic` 分派到多個型別明確的 inline 函式。

## 自我實作

我先驗證一般常數的結果，再用兩個都從 `3` 開始的變數，比較 macro 與 inline function 接收 `value++` 時的差異。完整程式也放在同目錄的 `example.c`：

```c
#include <stdio.h>

#define MIN(A, B) ((A) <= (B) ? (A) : (B))

static inline int min_int(int a, int b)
{
    return a <= b ? a : b;
}

int main(void)
{
    int macro_value = 3;
    int inline_value = 3;
    int limit = 10;
    int macro_result;
    int inline_result;

    printf("MIN(8, 3) = %d\n", MIN(8, 3));

    macro_result = MIN(macro_value++, limit);
    inline_result = min_int(inline_value++, limit);

    printf("macro: result=%d, value_after=%d\n",
           macro_result, macro_value);
    printf("inline: result=%d, value_after=%d\n",
           inline_result, inline_value);

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
MIN(8, 3) = 3
macro: result=4, value_after=5
inline: result=3, value_after=4
```

編譯器沒有產生 warning。

## 結果理解

第一行證明純值輸入時，`MIN(8, 3)` 正常得到 `3`。副作用測試則顯示：

- Macro 先用第一次 `macro_value++` 取得 `3` 做條件比較，原變數變成 `4`；因為 `3 <= 10`，true branch 又執行一次 `macro_value++`，這次 expression 的值是 `4`，原變數最後變成 `5`。所以結果是 `4`，而不是直覺期待的 `3`。
- Inline function 的 argument `inline_value++` 只在呼叫前求值一次，傳進函式的是 `3`，原變數只增加到 `4`；函式回傳 `min(3, 10)`，所以結果是 `3`。

這個輸出把「macro 可能重複求值」從一句規則變成可觀察的狀態差異。

## 嵌入式／平台注意事項

- 不要把 volatile memory-mapped register 直接傳給可能重複求值的 macro；多一次讀取就可能清除 status bit 或取得不同硬體狀態。
- `MIN(*p++, limit)` 可能讓指標前進兩次，造成漏資料或越界。
- Inline 是最佳化提示與語意工具，不等於標準保證一定不產生函式呼叫；是否展開仍由編譯器決定。
- 巨集 argument 不做一般函式式的型別限制。混用 signed 與 unsigned 時，仍要套用 usual arithmetic conversions。
- 某些 compiler extension 可用 temporary 保證單次求值，但會降低可攜性；純 ISO C 專案應優先用型別明確的 inline 函式。

## 小結

正確的傳統寫法是 `#define MIN(A, B) ((A) <= (B) ? (A) : (B))`，括號能處理 precedence，卻不能解決重複求值。只要 argument 可能有副作用，就先存成 temporary，或改用合適型別的 `static inline` 函式。

## 參考資料

- [RMB Consulting：原始第 2 題](https://rmbconsulting.us/publications/a-c-test-the-0x10-best-questions-for-would-be-embedded-programmers/#preprocessor)
- [ISO C11 working draft N1570，6.5.15 與 6.10.3](https://www.open-std.org/jtc1/sc22/wg14/www/docs/n1570.pdf)
