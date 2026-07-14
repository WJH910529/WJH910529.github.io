---
title: "[0x10-16] 詞法分析：a+++b 與 Maximum Munch"
date: 2026-07-14
category: [Notes]
tag: [firmware, embedded-c, interview, lexer]
---

# [0x10-16] 詞法分析：a+++b 與 Maximum Munch

## 原始來源

- [A 'C' Test: The 0x10 Best Questions for Would-be Embedded Programmers — Obfuscated Syntax](https://rmbconsulting.us/publications/a-c-test-the-0x10-best-questions-for-would-be-embedded-programmers/#obfuscated-syntax)

## 題目

下面的 C 程式是否合法？如果合法，執行後 `a`、`b`、`c` 各是多少？

```c
int a = 5, b = 7, c;

c = a+++b;
```

困難點不是加法，而是編譯器會如何把連續三個 `+` 切成 token。

## 先說結論／面試回答

這是合法的 C。依 lexical analysis 的 longest match（常稱 maximum munch）規則，`a+++b` 會切成：

```text
a  ++  +  b
```

也就是：

```c
c = a++ + b;
```

postfix `a++` 先提供遞增前的值 `5` 給加法，並安排 `a` 增加 1；因此 `c` 得到 `5 + 7 = 12`。完整 assignment 結束後，`a` 已變成 `6`，`b` 沒有改變，最後是：

```text
a = 6, b = 7, c = 12
```

這段程式雖合法，卻不值得在實務中使用；寫成 `a++ + b` 才能讓人直接看出 token 邊界。

## 觀念拆解

### 編譯器先切 token，再分析文法

C 編譯不是看到整行後直接猜數學意義。詞法階段先把字元拆成 identifier、operator、constant 等 preprocessing token，後續 parser 才依文法組成 expression。因此空白通常不改變已明確的 token，卻能大幅改善讀者理解。

### 為什麼不是 `a + ++b`

讀到 `a` 後，剩下 `+++b`。在目前位置，`++` 是比單一 `+` 更長、而且合法的 token，所以 lexer 先取 `++`；接著剩下的 `+` 才成為加法運算子。因此不會先切成 `+` 與 `++`。

### Post-increment 的值與副作用

`a++` 這個 expression 的值是遞增前的 `a`，而將 `a` 加 1 的副作用會在下一個 sequence point 前完成。本題沒有在同一個 full expression 的其他位置再次讀取 `a`，所以不存在未定義的 unsequenced read／write 衝突。assignment 這個 full expression 結束後再輸出 `a`，可觀察到它已是 `6`。

### 合法不等於好維護

難以閱讀的合法語法會增加 code review 負擔，也容易在後續修改時引入真正的 precedence 或 side-effect bug。原題用它來測詞法規則，同時也適合延伸討論 coding style、formatter、compiler warnings 與 static analysis。

## 自我實作

範例各跑一次緊密寫法與加上空白的等價寫法，再比較最終狀態。完整程式也放在同目錄的 `example.c`：

```c
#include <stdio.h>

int main(void)
{
    int a = 5;
    int b = 7;
    int c;
    int x = 5;
    int y = 7;
    int d;

    c = a+++b;
    d = x++ + y;

    printf("after c = a+++b: a=%d, b=%d, c=%d\n", a, b, c);
    printf("after d = x++ + y: x=%d, y=%d, d=%d\n", x, y, d);
    puts(a == x && b == y && c == d ? "same result? yes"
                                    : "same result? no");

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
after c = a+++b: a=6, b=7, c=12
after d = x++ + y: x=6, y=7, d=12
same result? yes
```

編譯器沒有產生 warning。

## 結果理解

第一行直接驗證 `a+++b` 的結果：`c` 使用遞增前的 `a`，所以是 `12`；statement 完成後 `a` 是 `6`。第二行把相同運算明確寫成 `x++ + y`，三個結果完全相同，最後印出 `same result? yes`。

這個實驗只能證明本題的 tokenization 與結果；不能推論所有塞在一起的 `+`／`-` 都合法。maximum munch 只負責取最長 token，切完後仍可能因 C 文法或型別規則而編譯失敗。

## 嵌入式／平台注意事項

- 本題結果不依賴 `int` 是 16 或 32 bits，因為 `5`、`7`、`12` 都在最小標準範圍內。
- 寄存器存取與 macro 常帶有副作用；把 `++` 放進複雜 expression 會讓硬體讀寫次數更難審查。
- 即使 expression 在標準上有定義，也要遵守團隊 coding standard；安全關鍵專案常限制 `++` 與其他運算子混在同一 expression。
- Formatter 能加入空白，但不能替代對副作用與 evaluation order 的理解；compiler warning 與 static analyzer 仍應啟用。
- 若讀者需要停下來重新切 token，維護成本已經過高。韌體程式碼應優先讓時間、狀態與硬體副作用一眼可見。

## 小結

`a+++b` 會依 maximum munch 切成 `a++ + b`，所以最後 `a=6`、`b=7`、`c=12`。它適合當面試題，不適合當產品程式碼；清楚的空白與簡單 expression 更重要。

## 參考資料

- [RMB Consulting：原始第 16 題](https://rmbconsulting.us/publications/a-c-test-the-0x10-best-questions-for-would-be-embedded-programmers/#obfuscated-syntax)
- [ISO C11 working draft N1570，5.1.1.2、6.4 與 6.5.2.4](https://www.open-std.org/jtc1/sc22/wg14/www/docs/n1570.pdf)
