---
title: "[0x10-03] 預處理器：#error 指令的用途"
date: 2026-07-14
category: [Notes]
tag: [firmware, embedded-c, interview, preprocessor]
---

# [0x10-03] 預處理器：`#error` 指令的用途

## 原始來源

- [A 'C' Test: The 0x10 Best Questions for Would-be Embedded Programmers — Preprocessor, Question 3](https://rmbconsulting.us/publications/a-c-test-the-0x10-best-questions-for-would-be-embedded-programmers/#preprocessor)

## 題目

說明預處理指令 `#error` 的用途，以及它適合用在什麼情境。

## 先說結論／面試回答

`#error` 會要求 C implementation 產生包含指定內容的診斷訊息。ISO C 並沒有進一步規定 compiler 必須立即終止，但 GCC、Clang 等常見工具鏈會把它當成 fatal error，使目前 translation unit 無法成功完成編譯。它通常搭配 `#if`、`#ifdef`、`#ifndef` 使用，在編譯期拒絕不支援或互相矛盾的設定，例如：

- 沒有選擇目標板或 MCU。
- 同時定義兩個互斥的硬體版本。
- 編譯器、CPU architecture 或 byte order 不受支援。
- 必要 feature macro、clock frequency 或 buffer size 沒有提供。

典型寫法如下：

```c
#if !defined(TARGET_DEMO_BOARD)
#error "TARGET_DEMO_BOARD must be defined"
#endif
```

重點是讓錯誤在 build 階段立即、清楚地失敗，而不是勉強產生一份設定不明的 firmware，等到燒錄後才用錯誤硬體行為暴露問題。

## 觀念拆解

### `#error` 何時生效

preprocessor 會先處理條件編譯。若包住 `#error` 的條件為 false，該分支不會要求診斷；若條件為 true，`#error` 會連同後面的 preprocessing tokens 形成診斷資訊。雖然 C 標準只明定必須產生 diagnostic，實務上的 build system 應把這個診斷視為失敗。

因此它很適合建立「沒有預設答案」的設定閘門：

```c
#if defined(BOARD_A)
/* BOARD_A configuration */
#elif defined(BOARD_B)
/* BOARD_B configuration */
#else
#error "Select BOARD_A or BOARD_B"
#endif
```

### `-D` 如何提供 build-time 設定

GCC 的 `-DNAME` 會在 preprocessing 前定義 `NAME`，效果近似在來源最前方寫 `#define NAME 1`。本篇先故意不加 `-D`，證明 `#error` 擋下 build；再加上 `-DTARGET_DEMO_BOARD`，讓條件變成 false 並成功編譯。

正式專案通常由 CMake、Makefile、IDE project 或 board support package 統一帶入這些定義，而不是要求每位開發者手動修改 `.c`。

### 與 `_Static_assert`、runtime `assert` 的差異

- `#error` 在 preprocessing 階段檢查 macro 與條件編譯選項，不需要 C 型別資訊。
- C11 `_Static_assert` 檢查 integer constant expression，適合驗證型別大小、結構 layout 或常數關係。
- `assert()` 是執行期檢查，且可能被 `NDEBUG` 關閉，不能取代不支援平台的 build-time gate。

三者處理的階段與問題不同，不是互相替代的同一功能。

## 自我實作

程式要求 build system 明確定義 `TARGET_DEMO_BOARD`。完整內容也放在同目錄的 `example.c`：

```c
#include <stdio.h>

#if !defined(TARGET_DEMO_BOARD)
#error "TARGET_DEMO_BOARD must be defined"
#endif

int main(void)
{
    puts("target: demo board");
    return 0;
}
```

這個實驗有兩條路徑：第一次不定義 macro，預期編譯失敗；第二次使用 `-D` 定義 macro，預期成功並執行程式。

## 編譯與執行指令

測試環境是 Windows 上的 **32-bit MinGW GCC 6.3.0 編譯目標**（`gcc -dumpmachine` 為 `mingw32`）。先執行未定義平台的失敗測試：

```powershell
gcc -std=c11 -Wall -Wextra -pedantic example.c -o example.exe
```

再由 command line 定義平台並執行：

```powershell
gcc -std=c11 -Wall -Wextra -pedantic -DTARGET_DEMO_BOARD example.c -o example.exe
.\example.exe
```

## 本機實際輸出

第一次未加 `-D`，GCC 的 exit code 是 `1`，實際診斷為：

```text
example.c:4:2: error: #error "TARGET_DEMO_BOARD must be defined"
 #error "TARGET_DEMO_BOARD must be defined"
  ^~~~~
```

這次失敗是測試預期結果，不是範例壞掉。第二次加入 `-DTARGET_DEMO_BOARD` 後，編譯 exit code 是 `0`、沒有 warning；執行 exit code 也是 `0`，輸出：

```text
target: demo board
```

## 結果理解

沒有定義 `TARGET_DEMO_BOARD` 時，`!defined(TARGET_DEMO_BOARD)` 為真，因此 preprocessor 遇到 `#error`，直接把清楚的原因放進 compiler diagnostic；本次 GCC invocation 以 exit code `1` 結束，沒有產生新的成功 build artifact。測試失敗路徑前仍應先清掉舊檔，因為先前產生的 `example.exe` 不會自動消失。

加上 `-DTARGET_DEMO_BOARD` 後，條件為 false，`#error` 所在分支被跳過。相同來源碼不需修改就能成功編譯並輸出 `target: demo board`。這證明平台選擇可以由 build system 注入，而 source code 負責驗證組態是否完整。

## 嵌入式／平台注意事項

- 多個互斥平台 macro 不只要檢查「至少一個」，也要檢查「不得同時多個」；否則 include 與 register 定義可能混用。
- 診斷文字應直接說明缺少什麼，以及預期由哪個 build option 提供，避免只寫模糊的 `#error error`。
- 不要用 `#error` 驗證只能在執行期得知的硬體狀態；它只能看到 preprocessing 階段的資訊。
- 編譯器內建 macro 名稱與語意可能不同，跨 toolchain 時要查 vendor 文件，並把相容判斷集中管理。
- CI 應同時測試支援組態能成功、錯誤組態會失敗，避免日後重構意外繞過 guard。
- Header 中的 `#error` 會影響所有 include 它的 translation unit；條件與訊息要足夠精準，避免讓合法使用情境也被擋下。

## 小結

`#error` 是編譯期的主動防線：當平台、工具鏈或 feature 組態不符合前提時，立刻用明確訊息停止 build。本機測試也證明同一份 source 會在缺少 macro 時失敗，加入正確 `-D` 後成功。

## 參考資料

- [RMB Consulting：原始第 3 題](https://rmbconsulting.us/publications/a-c-test-the-0x10-best-questions-for-would-be-embedded-programmers/#preprocessor)
- [ISO C11 working draft N1570，6.10.1 與 6.10.5](https://www.open-std.org/jtc1/sc22/wg14/www/docs/n1570.pdf)
- [GCC 6.3.0 Manual：Preprocessor Options (`-D`)](https://gcc.gnu.org/onlinedocs/gcc-6.3.0/gcc/Preprocessor-Options.html)
