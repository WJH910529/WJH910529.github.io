---
title: "[0x10-05] 資料宣告：讀懂指標、陣列與函式指標"
date: 2026-07-14
category: [Notes]
tag: [firmware, embedded-c, interview, pointer]
---

# [0x10-05] 資料宣告：讀懂指標、陣列與函式指標

## 原始來源

- [A 'C' Test: The 0x10 Best Questions for Would-be Embedded Programmers — Data Declarations](https://rmbconsulting.us/publications/a-c-test-the-0x10-best-questions-for-would-be-embedded-programmers/#data-declarations)

## 題目

使用識別字 `a`，寫出以下八種宣告：

1. 一個整數。
2. 一個指向整數的指標。
3. 一個指向「整數指標」的指標。
4. 一個包含 10 個整數的陣列。
5. 一個包含 10 個「整數指標」的陣列。
6. 一個指向「10 個整數之陣列」的指標。
7. 一個函式指標：函式接收一個 `int`，回傳 `int`。
8. 一個包含 10 個函式指標的陣列：每個函式都接收一個 `int`，回傳 `int`。

## 先說結論／面試回答

答案依序是：

```c
int a;                 /* (a) integer */
int *a;                /* (b) pointer to integer */
int **a;               /* (c) pointer to pointer to integer */
int a[10];             /* (d) array of 10 integers */
int *a[10];            /* (e) array of 10 pointers to integer */
int (*a)[10];          /* (f) pointer to array of 10 integers */
int (*a)(int);         /* (g) pointer to function(int) returning int */
int (*a[10])(int);     /* (h) array of 10 such function pointers */
```

我不只會背這八行，也會在面試時指出最容易混淆的兩組：

```c
int *a[10];    /* a 先和 [10] 結合：a 是陣列 */
int (*a)[10];  /* 括號讓 a 先和 * 結合：a 是指標 */
```

以及：

```c
int (*a)(int);       /* 一個函式指標 */
int (*a[10])(int);   /* 一個函式指標陣列 */
```

## 觀念拆解

### 從識別字開始向外讀

C 宣告可以從識別字 `a` 開始，先看和它結合得最緊的部分，再逐層向外讀。簡化後的優先順序是：

1. 括號內的宣告先處理。
2. 後綴 `[]` 與 `()` 比前綴 `*` 綁得更緊。
3. 最後再看最左邊的基礎型別。

例如：

```c
int *a[10];
```

`a` 右邊先遇到 `[10]`，所以 `a` 是「10 個元素的陣列」；再向外看到 `*`，得知每個元素是指標；最左邊的 `int` 表示每個指標都指向整數。

相對地：

```c
int (*a)[10];
```

括號迫使 `*a` 先結合，所以 `a` 先被判定為指標；括號外的 `[10]` 表示它指向的物件是一個 10 元素陣列。

### Array of pointers

`int *a[10]` 會在陣列裡保存 10 個位址。每個元素可以指向不同整數，也可以是 null pointer。修改 `*a[i]`，就是修改第 `i` 個指標指到的整數；這不是把整數複製進陣列。

常見用途包括字串表、物件表、scatter/gather buffer，以及多個硬體 channel 的控制區塊指標。

### Pointer to array

`int (*a)[10]` 的指標步長是一整列，也就是 `10 * sizeof(int)`。若 `a` 指向二維陣列的第一列，`++a` 會移到下一列，而不是移到同一列的下一個整數。

這個型別也保留了「每列剛好 10 個元素」的資訊，傳遞固定欄數的二維陣列時很有用。

### Function pointer

函式名稱在多數運算式中會轉成函式指標，因此可以寫：

```c
int (*a)(int) = twice;
```

呼叫時以下兩種形式等價：

```c
a(7);
(*a)(7);
```

函式指標常用於 callback、狀態機 action table、driver operation table 與中斷向量表。指標型別必須和實際函式的參數及回傳型別相容，不能只因為位址大小相同就任意轉型呼叫。

### Function pointer array

`int (*a[10])(int)` 先由 `a[10]` 確認它是陣列，再由 `*` 得知元素是指標，最後的 `(int)` 與左側 `int` 表示它們指向「接收 `int`、回傳 `int`」的函式。

它適合把命令編號或狀態直接對應到處理函式：

```c
result = a[command](argument);
```

實務上仍要先檢查索引範圍與元素是否為 null pointer。

## 自我實作

八種宣告不能在同一個 scope 同時使用相同名稱 `a`，因此範例用八個獨立 block，讓每個 block 都能重新宣告 `a`。完整程式放在同目錄的 `example.c`：

```c
#include <stdio.h>

static int twice(int value)
{
    return value * 2;
}

static int triple(int value)
{
    return value * 3;
}

static int square(int value)
{
    return value * value;
}

int main(void)
{
    {
        int a = 10;
        printf("(a) integer: %d\n", a);
    }

    {
        int value = 20;
        int *a = &value;
        printf("(b) pointer to integer: %d\n", *a);
    }

    {
        int value = 30;
        int *pointer = &value;
        int **a = &pointer;
        printf("(c) pointer to pointer: %d\n", **a);
    }

    {
        int a[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
        printf("(d) array of integers: a[0]=%d, a[9]=%d\n",
               a[0], a[9]);
    }

    {
        int first = 40;
        int second = 50;
        int third = 60;
        int *a[10] = { &first, &second, &third };

        ++*a[0];
        ++*a[1];
        ++*a[2];
        printf("(e) array of pointers: %d, %d, %d\n",
               *a[0], *a[1], *a[2]);
    }

    {
        int rows[2][10] = {
            { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 },
            { 11, 12, 13, 14, 15, 16, 17, 18, 19, 20 }
        };
        int (*a)[10] = rows;

        printf("(f) pointer to array: first row %d..%d\n",
               (*a)[0], (*a)[9]);
        ++a;
        printf("    after ++a: second row %d..%d\n",
               (*a)[0], (*a)[9]);
    }

    {
        int (*a)(int) = twice;
        printf("(g) function pointer: twice(7)=%d\n", a(7));
    }

    {
        int (*a[10])(int) = { twice, triple, square };

        printf("(h) function pointer array: %d, %d, %d\n",
               a[0](4), a[1](4), a[2](4));
    }

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
(a) integer: 10
(b) pointer to integer: 20
(c) pointer to pointer: 30
(d) array of integers: a[0]=0, a[9]=9
(e) array of pointers: 41, 51, 61
(f) pointer to array: first row 1..10
    after ++a: second row 11..20
(g) function pointer: twice(7)=14
(h) function pointer array: 8, 12, 16
```

編譯器沒有產生 warning。

## 結果理解

### `int *a[10]` 的結果

三個陣列元素分別保存 `first`、`second`、`third` 的位址。執行 `++*a[i]` 後，原本的 `40`、`50`、`60` 變成 `41`、`51`、`61`，證明改動的是指標指向的原始整數。

運算子解析也值得注意：

```c
++*a[0];
```

會先取 `a[0]`，再 dereference，最後遞增該整數；不是把陣列中的指標位址加一。

### `int (*a)[10]` 的結果

一開始 `a` 指向 `rows[0]`，所以 `(*a)[0]` 到 `(*a)[9]` 是 `1` 到 `10`。執行 `++a` 後，指標跨過完整的 10 個 `int`，改指向 `rows[1]`，因此讀到 `11` 到 `20`。

### 函式指標的結果

單一函式指標 `a` 指向 `twice`，所以 `a(7)` 得到 `14`。函式指標陣列的前三個元素分別指向 `twice`、`triple`、`square`，同樣輸入 `4` 後依序得到 `8`、`12`、`16`。這展示「資料決定呼叫哪個函式」，而不需要三段 `if`。

## 嵌入式／平台注意事項

- 陣列和指標是不同型別。陣列名稱常會轉成首元素指標，但 `sizeof(array)`、取址 `&array` 與函式參數調整等情況不能混為一談。
- 指標大小由 ABI 決定。本機 32-bit MinGW 指標為 4 bytes；其他 MCU 可能是 16、24、32 或 64 bits。
- Function pointer 和 object pointer 不保證能互相無損轉換；不要把資料位址直接當函式入口呼叫。
- MCU 的函式指標可能涉及不同 code space、bank 或 calling convention，宣告必須和 vendor ABI 相容。
- 函式指標表若來自外部輸入，必須先驗證索引；越界呼叫通常比一般資料越界更快造成控制流程劫持或 fault。
- 複雜宣告可以用 `typedef` 分段提升可讀性，但型別名稱不應刻意隱藏重要的 ownership、可空性或指標語意。

## 小結

這題真正測的是能不能從語法推出型別，而不是只背八行答案。先記住 `[]`、`()` 比 `*` 綁得更緊，再觀察括號如何改變結合順序，就能分清 pointer array、array pointer、function pointer 與 function pointer array。

## 參考資料

- [RMB Consulting：原始第 5 題](https://rmbconsulting.us/publications/a-c-test-the-0x10-best-questions-for-would-be-embedded-programmers/#data-declarations)
- [ISO C11 working draft N1570，6.7.6 Declarators](https://www.open-std.org/jtc1/sc22/wg14/www/docs/n1570.pdf)
- [繁中題目整理：韌體工程師應該要知道的 0x10 個問題](https://hackmd.io/GqAzU2LpRbqapcnNYUpEJw)
