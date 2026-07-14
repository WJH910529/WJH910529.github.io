---
title: "[0x10-01] 預處理器：一年有幾秒？"
date: 2026-07-14
category: [Notes]
tag: [firmware, embedded-c, interview, preprocessor]
---

# [0x10-01] 預處理器：一年有幾秒？

## 原始來源

- [A 'C' Test: The 0x10 Best Questions for Would-be Embedded Programmers — Preprocessor, Question 1](https://rmbconsulting.us/publications/a-c-test-the-0x10-best-questions-for-would-be-embedded-programmers/#preprocessor)

## 題目

使用 `#define` 寫出一個巨集 `SECONDS_PER_YEAR`，表示一年共有多少秒。題目不考慮閏年。

## 面試回答

```c
#define SECONDS_PER_YEAR (60UL * 60UL * 24UL * 365UL)
```

計算結果為：

```text
60 × 60 × 24 × 365 = 31,536,000
```

這個答案除了數字正確，也刻意處理了名稱、括號、分號與整數運算型別等細節。

## 觀念拆解

### 1. `#define` 不是變數宣告

`SECONDS_PER_YEAR` 是一段會在前處理階段展開的 replacement list，不會因此配置一塊可讀寫的記憶體。使用全大寫名稱，是 C 專案中常見的常數巨集慣例。

### 2. 巨集定義尾端不加分號

以下寫法不理想：

```c
#define SECONDS_PER_YEAR (60UL * 60UL * 24UL * 365UL);
```

分號會一起被展開。像 `_Static_assert(SECONDS_PER_YEAR == 31536000UL, "...");` 這類需要 expression 的位置，就會因多出分號而造成語法錯誤。

### 3. 為什麼整個運算式要加括號？

巨集只是文字替換。若未加外層括號，放入其他運算式時可能因運算子優先順序而改變原意。常數巨集雖然比帶參數巨集單純，仍建議把完整 expression 包起來：

```c
#define SECONDS_PER_YEAR (60UL * 60UL * 24UL * 365UL)
```

### 4. `UL` 防止中途以過小的 `int` 計算

沒有 suffix 的十進位整數常值，會先選擇可容納它的 signed integer type；這裡的 `60`、`24` 與 `365` 通常都是 `int`。因此：

```c
#define BAD_SECONDS_PER_YEAR (60 * 60 * 24 * 365)
```

每一步乘法都可能先以 `int` 計算。在常見的 32-bit `int` 主機上，`31,536,000` 還在範圍內；但若目標平台的 `int` 只有 16 bits，運算會在得到最後結果之前溢位。事後再把錯誤結果存入 `unsigned long` 並不能補救。

把第一個 operand 寫成 `60UL`，就足以透過 usual arithmetic conversions 讓後續乘法使用 `unsigned long`；全部都標示 `UL` 則更直接，也更容易閱讀：

```c
#define SECONDS_PER_YEAR (60UL * 60UL * 24UL * 365UL)
```

注意，suffix 是整數常值語法的一部分，所以應寫成 `365UL`，不能把它接在括號後面：

```c
/* 錯誤：這不是合法的 C 語法 */
(60 * 60 * 24 * 365)UL
```

C 標準保證 `unsigned long` 至少可表示到 `4,294,967,295`，因此能容納 `31,536,000`。

### 5. 用編譯期斷言保護常數

C11 的 `_Static_assert` 能在編譯時確認巨集值。如果未來有人改壞運算式，錯誤會在 build 階段出現，而不是等程式執行後才發現：

```c
_Static_assert(SECONDS_PER_YEAR == 31536000UL,
               "SECONDS_PER_YEAR has an unexpected value");
```

## 自我實作

以下程式除了印出巨集值，也觀察本次測試環境的 `int`、`unsigned long` 大小與 `ULONG_MAX`。

```c
#include <limits.h>
#include <stdio.h>

#define SECONDS_PER_YEAR (60UL * 60UL * 24UL * 365UL)

_Static_assert(SECONDS_PER_YEAR == 31536000UL,
               "SECONDS_PER_YEAR has an unexpected value");

int main(void)
{
    printf("SECONDS_PER_YEAR = %lu\n", SECONDS_PER_YEAR);
    printf("sizeof(int) = %lu bytes\n", (unsigned long)sizeof(int));
    printf("sizeof(unsigned long) = %lu bytes\n",
           (unsigned long)sizeof(unsigned long));
    printf("ULONG_MAX = %lu\n", ULONG_MAX);

    return 0;
}
```

完整程式另存於文章同目錄的 `example.c`，並已在上方完整列出。

## 編譯與執行

本系列實測使用 MinGW.org GCC 6.3.0，以 C11 與常用警告選項編譯：

```powershell
gcc -std=c11 -Wall -Wextra -pedantic example.c -o example.exe
.\example.exe
```

### 本機實際輸出

```text
SECONDS_PER_YEAR = 31536000
sizeof(int) = 4 bytes
sizeof(unsigned long) = 4 bytes
ULONG_MAX = 4294967295
```

程式正常結束，編譯器沒有產生警告。

## 結果理解

- 實際輸出的 `31,536,000` 與手算及 `_Static_assert` 的預期一致。
- 本機的 `int` 是 4 bytes，未加 suffix 的版本在這台機器上也不會溢位；這不代表它對所有嵌入式平台都安全。
- 本機的 `unsigned long` 也是 4 bytes，最大值為 `4,294,967,295`，足以容納一年秒數。
- `UL` 的目的不是讓這台主機「剛好跑對」，而是明確指定運算型別，避免換到較窄的 `int` 平台時出錯。

## 嵌入式與平台注意事項

1. 題目明確忽略閏年；若是實際日曆、RTC 或憑證期限，不能把每一年都視為固定 365 天。
2. `unsigned long` 的位元寬度由實作決定，本機結果不能直接代表 MCU；應查看 compiler ABI、`limits.h` 或以 static assertion 驗證。
3. 如果專案需要固定寬度，可改用 `<stdint.h>` 的型別與 `UINT32_C`，但仍要注意整個 expression 的型別，而不只是最後儲存的變數型別。
4. 真實韌體也常以 timer tick 或 clock frequency 換算時間；乘法順序與中間值範圍同樣需要檢查。

## 小結

這題表面上只是算術，實際在確認候選人是否理解前處理器、巨集寫法、integer literal suffix，以及「溢位發生在 expression 評估期間」這件事。穩健的答案是：

```c
#define SECONDS_PER_YEAR (60UL * 60UL * 24UL * 365UL)
```

## 參考資料

- [A 'C' Test: The 0x10 Best Questions for Would-be Embedded Programmers](https://rmbconsulting.us/publications/a-c-test-the-0x10-best-questions-for-would-be-embedded-programmers/)
- [ISO/IEC 9899:2011 Committee Draft N1570](https://www.open-std.org/jtc1/sc22/wg14/www/docs/n1570.pdf)
- [C 語言測試：應知道的 0x10 個基本問題](https://hackmd.io/GqAzU2LpRbqapcnNYUpEJw)
- [工程師應知道的 0x10 個問題：總覽](https://medium.com/@racktar7743/%E5%B7%A5%E7%A8%8B%E5%B8%AB%E6%87%89%E7%9F%A5%E9%81%93%E7%9A%840x10%E5%80%8B%E5%95%8F%E9%A1%8C-%E7%B8%BD%E8%A6%BD-970cc31f6b76)
