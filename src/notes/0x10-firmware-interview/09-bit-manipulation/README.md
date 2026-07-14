---
title: "[0x10-09] 位元操作：設定與清除指定 Bit"
date: 2026-07-14
category: [Notes]
tag: [firmware, embedded-c, interview, bitwise]
---

# [0x10-09] 位元操作：設定與清除指定 Bit

## 原始來源

- [A 'C' Test: The 0x10 Best Questions for Would-be Embedded Programmers — Bit Manipulation](https://rmbconsulting.us/publications/a-c-test-the-0x10-best-questions-for-would-be-embedded-programmers/#bit-manipulation)

## 題目

給定整數變數 `a`，寫出兩段程式：第一段設定 bit 3，第二段清除 bit 3；兩種操作都不能改變其他位元。

這裡依 C 與硬體文件常見習慣，bit 編號從 0 開始，因此 bit 3 的 mask 是：

```text
1 << 3 = 0b00001000 = 0x08
```

## 先說結論／面試回答

我會先定義 bit mask，再使用 read-modify-write：

```c
#define BIT3 (1u << 3)

a |= BIT3;     /* set bit 3 */
a &= ~BIT3;    /* clear bit 3 */
```

設定時用 OR，因為任何位元和 `0` 做 OR 都保持原值，只有 mask 中的 bit 3 是 `1`，會被強制設成 `1`。

清除時先把 mask 取補數，得到「bit 3 是 0、其餘全是 1」的值，再做 AND。任何位元和 `1` 做 AND 都保持原值，只有 bit 3 會被強制清成 `0`。

## 觀念拆解

### 設定位元：OR

單一位元的 OR 真值是：

| 原值 | Mask | 結果 |
|---:|---:|---:|
| 0 | 0 | 0 |
| 1 | 0 | 1 |
| 0 | 1 | 1 |
| 1 | 1 | 1 |

因此 `a | BIT3` 只會影響 mask 為 1 的 bit 3，其餘位置的 mask 都是 0，原值會保留。`a |= BIT3` 是相同運算的 compound assignment 寫法。

### 清除位元：AND 與補數

若直接寫 `a &= BIT3`，結果會保留 bit 3、清掉其他所有 bit，剛好和需求相反。正確作法是先取補數：

```text
BIT3  = 00001000
~BIT3 = 11110111   （只畫低 8 bits）
```

再以 `a &= ~BIT3` 讓 bit 3 和 0 做 AND，其他 bit 則和 1 做 AND。

### Mask 的型別也重要

範例使用：

```c
#define BIT(n) (UINT32_C(1) << (n))
```

依 `<stdint.h>` 的規則，`UINT32_C(1)` 會產生適合 `uint_least32_t`、並經 integer promotion 後之型別的 unsigned integer constant expression，而不是保證 expression 型別名稱剛好就是 `uint32_t`。它至少讓本例不會從 signed `1` 開始位移；若移到 signed 符號位，或位移量大於等於左運算元型別寬度，可能產生未定義行為。通用 `BIT(n)` 仍必須保證 `n < 32`。

## 自我實作

我從 `0xA5` 開始。它的低 8 bits 是 `10100101`，bit 3 原本為 0；設定後應變成 `10101101`（`0xAD`），再清除則回到 `10100101`（`0xA5`）。完整程式放在同目錄的 `example.c`：

```c
#include <stdint.h>
#include <stdio.h>

#define BIT(n) (UINT32_C(1) << (n))
#define BIT3 BIT(3)

static void print_value(const char *label, uint32_t value)
{
    unsigned int bit;

    printf("%-13s 0x%02lX  ", label, (unsigned long)value);
    for (bit = 8; bit > 0; --bit) {
        putchar((value & BIT(bit - 1)) != 0U ? '1' : '0');
    }
    putchar('\n');
}

int main(void)
{
    const uint32_t original = UINT32_C(0xA5);
    uint32_t value = original;
    int other_bits_unchanged;

    print_value("initial:", value);

    value |= BIT3;
    print_value("set bit 3:", value);

    other_bits_unchanged =
        (value & ~BIT3) == (original & ~BIT3);

    value &= ~BIT3;
    print_value("clear bit 3:", value);

    other_bits_unchanged = other_bits_unchanged &&
        ((value & ~BIT3) == (original & ~BIT3));
    printf("other bits unchanged: %s\n",
           other_bits_unchanged ? "yes" : "no");

    return 0;
}
```

除了顯示十六進位與二進位，程式還把 bit 3 遮掉後比較操作前後的值，直接驗證其他 bits 沒有變化。

## 編譯與執行指令

測試環境是 Windows 上的 **32-bit MinGW GCC 6.3.0 編譯目標**（`gcc -dumpmachine` 為 `mingw32`）：

```powershell
gcc -std=c11 -Wall -Wextra -pedantic example.c -o example.exe
.\example.exe
```

## 本機實際輸出

```text
initial:      0xA5  10100101
set bit 3:    0xAD  10101101
clear bit 3:  0xA5  10100101
other bits unchanged: yes
```

編譯器沒有產生 warning。

## 結果理解

`0xA5 | 0x08` 得到 `0xAD`，二進位只有從右數第 4 個位置由 0 變成 1。接著 `0xAD & ~0x08` 回到 `0xA5`。最後的 `yes` 不是只比較完整數值，而是明確遮掉 bit 3 後比較其他位元，因此驗證了題目的「其他 bits 保持不變」。

設定已經為 1 的 bit，或清除已經為 0 的 bit，結果也不會額外變化；這兩個操作對目標 bit 都具有 idempotent 性質。

## 嵌入式／平台注意事項

- Datasheet 可能從 bit 0 開始編號，也可能用欄位名稱而不是數字；實作前先確認文件定義。
- Hardware register 通常要透過 `volatile` 存取，但 `volatile` 不會讓 read-modify-write 變成原子操作。若 ISR、另一核心或硬體會同時修改 register，可能發生 lost update。
- 有些 MCU 提供 atomic SET／CLEAR register 或 bit-banding，應優先使用硬體設計的介面。
- 某些 status register 採 write-one-to-clear（W1C）。對它執行一般 `a &= ~mask` 可能清錯旗標，必須依 datasheet 寫入指定 mask。
- 不要對 signed 值做可能碰到符號位的左移；固定寬度 register 可使用 `<stdint.h>` 與相符的常值巨集。
- 多 bit 欄位要先清除欄位 mask，再把限制寬度後的新值 shift 進去，不能直接套用單一 bit 的寫法。

## 小結

設定 bit 使用 `|= mask`，清除 bit 使用 `&= ~mask`。面試回答到這裡只是基本分；完整理解還包括 mask 型別、合法位移範圍、register 的 W1C 語意，以及 read-modify-write 的並行風險。

## 參考資料

- [RMB Consulting：原始第 9 題](https://rmbconsulting.us/publications/a-c-test-the-0x10-best-questions-for-would-be-embedded-programmers/#bit-manipulation)
- [Medium：工程師應知道的 0x10 個問題（9）— Bit 操作](https://medium.com/@racktar7743/%E5%B7%A5%E7%A8%8B%E5%B8%AB%E6%87%89%E7%9F%A5%E9%81%93%E7%9A%840x10%E5%80%8B%E5%95%8F%E9%A1%8C-9-bit%E6%93%8D%E4%BD%9C-f119a38eeba6)
- [ISO C11 working draft N1570，6.5.7 Bitwise shift operators](https://www.open-std.org/jtc1/sc22/wg14/www/docs/n1570.pdf)
