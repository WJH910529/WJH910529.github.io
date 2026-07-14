---
title: "[0x10-15] 型別別名：typedef 與 #define 的差異"
date: 2026-07-14
category: [Notes]
tag: [firmware, embedded-c, interview, typedef]
---

# [0x10-15] 型別別名：typedef 與 #define 的差異

## 原始來源

- [A 'C' Test: The 0x10 Best Questions for Would-be Embedded Programmers — Typedef](https://rmbconsulting.us/publications/a-c-test-the-0x10-best-questions-for-would-be-embedded-programmers/#typedef)

## 題目

以下兩種寫法都想把名稱定義成「指向 `struct s` 的指標」。哪一種比較好？為什麼？

```c
#define dPS struct s *
typedef struct s *tPS;
```

接著考慮：

```c
dPS p1, p2;
tPS p3, p4;
```

四個變數是否都是指標？

## 先說結論／面試回答

若只能在題目兩種方法中選擇，應選 `typedef`。`#define` 只做 preprocessing token 的文字替換，因此：

```c
dPS p1, p2;
```

展開後是：

```c
struct s *p1, p2;
```

`*` 屬於個別 declarator，所以只有 `p1` 是指標，`p2` 是一個 `struct s` 物件。相對地，`tPS` 是編譯器認得的完整型別別名，因此 `tPS p3, p4;` 會讓 `p3` 與 `p4` 都成為指標。

不過在實務程式中，我會再考慮「pointer typedef 是否隱藏了間接層級」。常見且更清楚的替代方案是只替 struct 本體取別名，再顯式寫星號：

```c
typedef struct s S;
S *p3;
S *p4;
```

這不改變本題答案：`typedef` 的語意仍比用 macro 假裝型別可靠。

## 觀念拆解

### 宣告中的星號屬於 declarator

在 `struct s *p1, p2;` 中，共用的 declaration specifier 是 `struct s`，而 `*p1` 與 `p2` 是兩個不同 declarator。這和下面的經典陷阱相同：

```c
int *a, b; /* a 是 int *，b 是 int */
```

把 `*` 藏進 macro 並不會改變 C 的宣告文法。

### Macro 不知道什麼是型別

preprocessor 在 C parser 之前運作。它只看到 token 並替換，沒有型別檢查，也不會把 `struct s *` 封裝成不可分割的型別單位。`typedef` 則會把名稱登記為 typedef name，後續宣告由編譯器按型別規則解析。

### `const` 也會暴露兩者差異

例如：

```c
const dPS a; /* 展開為 const struct s *a：指向唯讀 struct 的指標 */
const tPS b; /* b 是 const pointer，近似 struct s * const b */
```

同樣的文字外觀有不同意義，是 pointer macro 很難維護的另一個原因。pointer typedef 雖正確，仍可能讓讀者看不出 `const` 修飾的是指標，因此命名規範與使用場合要一致。

## 自我實作

範例使用 C11 `_Generic` 在編譯期依 expression 型別選出文字，並用 `.`／`->` 分別存取實際物件。完整程式也放在同目錄的 `example.c`：

```c
#include <stdio.h>

struct s {
    int value;
};

#define dPS struct s *
typedef struct s *tPS;

#define TYPE_NAME(value) _Generic((value), \
    struct s *: "pointer to struct s",       \
    struct s: "struct s")

int main(void)
{
    struct s first = {10};
    struct s third = {30};
    struct s fourth = {40};
    dPS p1, p2;
    tPS p3, p4;

    p1 = &first;
    p2.value = 20;
    p3 = &third;
    p4 = &fourth;

    printf("p1: %s, value=%d\n", TYPE_NAME(p1), p1->value);
    printf("p2: %s, value=%d\n", TYPE_NAME(p2), p2.value);
    printf("p3: %s, value=%d\n", TYPE_NAME(p3), p3->value);
    printf("p4: %s, value=%d\n", TYPE_NAME(p4), p4->value);

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
p1: pointer to struct s, value=10
p2: struct s, value=20
p3: pointer to struct s, value=30
p4: pointer to struct s, value=40
```

編譯器沒有產生 warning。

## 結果理解

`p1` 顯示為 pointer，`p2` 顯示為 struct，證明 `dPS p1, p2` 並沒有建立兩個指標。程式也只能對 `p1` 使用 `->`、對 `p2` 使用 `.`；若寫反，編譯器會報型別錯誤。

`p3` 與 `p4` 則都顯示為 pointer，因為 `tPS` 整體已是 `struct s *` 的 typedef name。實驗不是靠 `sizeof` 猜型別，而是讓編譯器的 `_Generic` 直接依型別選擇結果。

## 嵌入式／平台注意事項

- 硬體 SDK 常用 typedef 隔離 ABI 型別或宣告 opaque handle；先確認名稱代表 object、pointer 還是整數 handle。
- pointer typedef 可能隱藏 `const`、`volatile` 修飾的位置。若操作 memory-mapped register，修飾錯一層可能改變存取語意。
- 不要用 macro 假裝複雜型別；macro 適合條件編譯或常數替換，型別交給 `typedef` 與編譯器。
- 公開 API 的 typedef 仍受 ABI 影響。更換底層型別後，要重新檢查大小、對齊、calling convention 與 binary compatibility。
- 宣告多個指標時可每行只宣告一個，降低 `int *a, b;` 類錯誤的閱讀成本。

## 小結

`#define` 是文字替換，`typedef` 是型別別名；兩者看似都能縮短宣告，實際語意並不等價。本題的 `dPS p1, p2` 只有一個指標，而 `tPS p3, p4` 才有兩個指標。

## 參考資料

- [RMB Consulting：原始第 15 題](https://rmbconsulting.us/publications/a-c-test-the-0x10-best-questions-for-would-be-embedded-programmers/#typedef)
- [ISO C11 working draft N1570，6.7.8 與 6.10.3](https://www.open-std.org/jtc1/sc22/wg14/www/docs/n1570.pdf)
