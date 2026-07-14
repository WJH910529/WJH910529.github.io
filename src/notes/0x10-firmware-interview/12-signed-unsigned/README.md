---
title: "[0x10-12] 整數轉型：Signed 與 Unsigned 混合運算"
date: 2026-07-14
category: [Notes]
tag: [firmware, embedded-c, interview, integer-conversion]
---

# [0x10-12] 整數轉型：Signed 與 Unsigned 混合運算

## 原始來源

- [A 'C' Test: The 0x10 Best Questions for Would-be Embedded Programmers — Code Examples, Question 12](https://rmbconsulting.us/publications/a-c-test-the-0x10-best-questions-for-would-be-embedded-programmers/#code-examples)

## 題目

下面的程式會輸出什麼？為什麼？

```c
void foo(void)
{
    unsigned int a = 6;
    int b = -20;

    (a + b > 6) ? puts("> 6") : puts("<= 6");
}
```

直覺可能先算出 `6 + (-20) = -14`，再回答 `<= 6`；但 C 的型別轉換規則會讓實際答案不同。

## 先說結論／面試回答

在原題常見的前提，也就是 `int` 與 `unsigned int` 具有相同 rank，且 `int` 無法表示 `unsigned int` 的全部值時，`b` 會先轉成 `unsigned int`。因此負數 `-20` 變成一個很大的無號值，整個加法也以無號運算進行，判斷結果為真，輸出：

```text
> 6
```

在本機 32-bit `unsigned int` 上：

- `(unsigned int)-20` 是 `4294967276`。
- `6u + 4294967276u` 依模數 `2^32` 運算，得到 `4294967282`。
- `4294967282 > 6u` 為真。

更精確地說，不是所有 signed／unsigned 混合運算都「一律轉 unsigned」；要依 integer promotions、兩邊的 conversion rank，以及 signed 型別能否涵蓋 unsigned 型別的全部值，套用 usual arithmetic conversions。

## 觀念拆解

### Integer promotions

小於 `int` rank 的整數型別，例如多數平台的 `char` 與 `short`，通常會先提升成 `int`；若 `int` 表示不了其全部值，才提升為 `unsigned int`。本題兩個運算元本來就是 `int` 與 `unsigned int`，不需要再做這一步，但面試時應知道整套規則從 promotions 開始。

### Usual arithmetic conversions

`a` 是 `unsigned int`，`b` 是 `int`，兩者 rank 相同。在本機 `UINT_MAX` 大於 `INT_MAX`，所以 `int` 無法涵蓋所有 `unsigned int` 值，`b` 會轉成 `unsigned int`，運算結果型別也是 `unsigned int`。

### 負數轉無號數

轉成 N-bit 無號型別時，結果會落在 `0` 到 `2^N - 1`，等價於反覆加減 `2^N` 直到進入範圍。因此 32-bit 環境的 `-20` 轉換結果是：

```text
2^32 - 20 = 4294967276
```

接著再加 `6`，得到 `4294967282`，沒有發生 signed overflow；這是定義良好的 unsigned modular arithmetic。

## 自我實作

除了重現原始分支，我把轉型後的中間值全部印出。完整程式也放在同目錄的 `example.c`：

```c
#include <limits.h>
#include <stdio.h>

int main(void)
{
    unsigned int a = 6;
    int b = -20;
    unsigned int converted_b = (unsigned int)b;
    unsigned int sum = a + b;

    printf("unsigned int object bits = %u\n",
           (unsigned int)(sizeof(unsigned int) * CHAR_BIT));
    printf("UINT_MAX = %u\n", UINT_MAX);
    printf("b as unsigned int = %u\n", converted_b);
    printf("a + b as unsigned int = %u\n", sum);
    (a + b > 6) ? puts("comparison result: > 6")
                : puts("comparison result: <= 6");

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
b as unsigned int = 4294967276
a + b as unsigned int = 4294967282
comparison result: > 6
```

編譯器沒有產生 warning。

## 結果理解

`sizeof(unsigned int) * CHAR_BIT` 先顯示本機 object representation 共有 32 bits；單看這個值理論上可能包含 padding bits，但本機同時輸出的 `UINT_MAX = 4294967295` 證明其值域確實使用 32 個 value bits。`b` 的數學值雖是 `-20`，一旦依運算規則轉為 `unsigned int`，位於無號範圍中的對應值就是 `4294967276`；和 `a` 相加後得到 `4294967282`，所以條件成立。

若單看變數宣告後直接用心算，很容易忽略「比較前已完成型別轉換」。追 C 運算式時，應先決定每個運算元轉成什麼型別，再計算值。

## 嵌入式／平台注意事項

- 不要假設 `int` 永遠是 32 bits；16-bit MCU 上常見 16-bit `int`，實際數值會改變，但在同 rank 的典型情況下分支仍可能是 `> 6`。
- register、封包長度與計時差值常用無號型別，混入 signed 錯誤碼時特別容易發生這類 bug。
- 編譯時啟用轉換相關警告（例如 GCC 的 `-Wsign-conversion`）有助於找出風險；本篇依指定命令只使用 `-Wall -Wextra -pedantic`。
- 若需求是帶正負號的數學運算，應先驗證範圍，再顯式轉到能表示兩邊值的 signed 型別；不要靠隱式轉換猜結果。
- 固定寬度資料可考慮 `<stdint.h>`，但仍須留意整數提升會把較窄型別提升成 `int` 或 `unsigned int`。

## 小結

本題的關鍵是先判斷運算型別，再判斷數值。signed 與 unsigned 混用不一定錯，但若沒有清楚掌握 usual arithmetic conversions，程式的比較結果很可能和直覺相反。

## 參考資料

- [RMB Consulting：原始第 12 題](https://rmbconsulting.us/publications/a-c-test-the-0x10-best-questions-for-would-be-embedded-programmers/#code-examples)
- [ISO C11 working draft N1570，6.3.1.3 與 6.3.1.8](https://www.open-std.org/jtc1/sc22/wg14/www/docs/n1570.pdf)
