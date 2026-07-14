---
title: "[0x10-07] const：唯讀語意與指標宣告"
date: 2026-07-14
category: [Notes]
tag: [firmware, embedded-c, interview, const]
---

# [0x10-07] `const`：唯讀語意與指標宣告

## 原始來源連結

- [A 'C' Test — Const, Question 7](https://rmbconsulting.us/publications/a-c-test-the-0x10-best-questions-for-would-be-embedded-programmers/#const)

## 題目

`const` 關鍵字代表什麼？並解釋下列宣告：

```c
const int a;
int const a;
const int *a;
int * const a;
int const * const a;
```

## 先說結論／面試回答

`const` 比「數值永遠不變」更精確的說法是：**不能透過 const-qualified lvalue 修改該物件**。它為編譯器與讀程式的人表達唯讀介面，並讓誤寫在編譯階段被抓出來。

| 宣告 | 意思 | 可改指標指向？ | 可透過它改資料？ |
| --- | --- | --- | --- |
| `const int a` | 唯讀的 `int` | 不適用 | 否 |
| `int const a` | 與上一列完全相同 | 不適用 | 否 |
| `const int *a` | 指向唯讀 `int` 的指標 | 是 | 否 |
| `int * const a` | 指向 `int` 的唯讀指標 | 否 | 是 |
| `int const * const a` | 指向唯讀 `int` 的唯讀指標 | 否 | 否 |

原題列的是辨識型別用的宣告片段。實際在函式內定義 const pointer 時，應立即初始化，例如 `int * const p = &value;`。

## 觀念拆解

### 從識別字向外閱讀

以 `const int * const p = &value;` 為例：

1. 從 `p` 開始看。
2. `p` 旁的 `const` 表示指標本身不能重新指定。
3. `*` 表示 `p` 是指標。
4. 左邊的 `const int` 表示被指向的整數也不能透過 `p` 修改。

所以 `p = &other;` 與 `*p = 10;` 都不允許。

### Pointer to const 不等於原物件永遠不變

```c
int value = 10;
const int *p = &value;

value = 20;  /* 合法：透過原本的非 const 名稱修改 */
/* *p = 20;     不合法：不能透過 p 修改 */
```

`p` 提供唯讀觀看方式；原物件若不是以 const-qualified type 定義，仍可能經其他合法路徑改變。反過來，若物件本身就是 `const int value`，強制轉型後寫入它是 undefined behavior，不是合法技巧。

### `const` 不一定是編譯期常數或 ROM

C 的 `const int n = 10;` 是唯讀物件，但不會自動在所有語境成為 integer constant expression。它實際放在 ROM、Flash 或 RAM，由儲存期、工具鏈、linker script 與平台決定。

函式宣告為 `int sum(const int *data, size_t count)`，則是在告訴呼叫者與編譯器：函式不會透過 `data` 修改陣列。const correctness 能縮小副作用，也讓錯誤修改成為編譯錯誤。

## 自我實作

以下程式操作三種常見指標型態，並把陣列以 pointer-to-const 傳入函式。完整程式存放在同目錄的 `example.c`。

```c
#include <stddef.h>
#include <stdio.h>

static int sum_samples(const int *samples, size_t count)
{
    size_t index;
    int total = 0;

    for (index = 0; index < count; ++index) {
        total += samples[index];
    }

    return total;
}

int main(void)
{
    int first = 10;
    int second = 20;
    const int limit = 100;
    const int *pointer_to_const = &first;
    int * const const_pointer = &first;
    const int * const const_pointer_to_const = &second;
    int samples[] = { 3, 5, 7 };

    printf("limit: %d\n", limit);

    printf("pointer_to_const -> %d\n", *pointer_to_const);
    pointer_to_const = &second;
    printf("after pointer move -> %d\n", *pointer_to_const);

    *const_pointer = 11;
    printf("value changed through const pointer: %d\n", first);

    printf("const pointer to const data -> %d\n",
           *const_pointer_to_const);
    printf("sum of read-only samples: %d\n",
           sum_samples(samples, sizeof samples / sizeof samples[0]));

    return 0;
}
```

若取消下列任一行的註解，compiler 就應拒絕：

```c
/* *pointer_to_const = 99; */       /* 被指向的資料是唯讀介面 */
/* const_pointer = &second; */      /* 指標本身是 const */
/* *const_pointer_to_const = 99; */ /* 資料與指標都不能改 */
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
limit: 100
pointer_to_const -> 10
after pointer move -> 20
value changed through const pointer: 11
const pointer to const data -> 20
sum of read-only samples: 15
```

## 結果理解

- `pointer_to_const` 可以從 `first` 改指向 `second`，但不能透過它寫入整數。
- `const_pointer` 自己不能改指向別處，但它指向一般 `int`，所以 `*const_pointer = 11` 合法，`first` 真的變成 11。
- `const_pointer_to_const` 的指標與資料兩層都受限，程式只能讀出 20。
- `sum_samples()` 只讀取陣列並得到 15。`const int *` 把「不改輸入」寫進函式介面，不需要複製整個陣列。

## 嵌入式／平台注意事項

- `const` 不保證物件一定放在 Flash。是否占用 RAM、是否需要從映像複製，以及有無硬體保護，都要看 compiler、linker script 與 memory map。
- 大型查表資料常宣告為 `static const`，但某些 MCU 還需要 section attribute 或供應商語法，才能確保只留在程式記憶體。
- 硬體唯讀狀態暫存器可能適合 `const volatile`：軟體不應寫入，所以是 `const`；硬體會改變，所以是 `volatile`。
- 只有原物件本來不是 const-qualified definition 時，移除 qualifier 後經正確型別別名修改才可能合法；寫入真正以 `const` 定義的物件是 undefined behavior，還可能觸發唯讀記憶體錯誤。
- API 若接受字串常值或唯讀 buffer，參數型別應保留 `const`，不要迫使呼叫端做不安全的 cast。

## 小結

面試時不要只回答「`const` 就是常數」。較準確的答案是：`const` 建立不能透過該 lvalue 修改的唯讀介面。判讀指標宣告時，要分清楚是資料被修飾、指標本身被修飾，還是兩者都被修飾，並在實際 definition 正確初始化 const pointer。

## 參考資料

- [RMB Consulting — A 'C' Test](https://rmbconsulting.us/publications/a-c-test-the-0x10-best-questions-for-would-be-embedded-programmers/#const)
- [ISO/IEC 9899:201x Committee Draft N1570（6.5.16、6.7.3）](https://www.open-std.org/jtc1/sc22/wg14/www/docs/n1570.pdf)
- [SEI CERT C — EXP05-C. Do not cast away a const qualification](https://wiki.sei.cmu.edu/confluence/display/c/EXP05-C.+Do+not+cast+away+a+const+qualification)
- [0x10 題繁中整理](https://hackmd.io/GqAzU2LpRbqapcnNYUpEJw)
- [Medium 系列總覽：工程師應知道的 0x10 個問題](https://medium.com/@racktar7743/%E5%B7%A5%E7%A8%8B%E5%B8%AB%E6%87%89%E7%9F%A5%E9%81%93%E7%9A%840x10%E5%80%8B%E5%95%8F%E9%A1%8C-%E7%B8%BD%E8%A6%BD-970cc31f6b76)
