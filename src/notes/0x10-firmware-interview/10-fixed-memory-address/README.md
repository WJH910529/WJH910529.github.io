---
title: "[0x10-10] 固定記憶體位址：存取 Memory-Mapped 資料"
date: 2026-07-14
category: [Notes]
tag: [firmware, embedded-c, interview, memory-mapped-io]
---

# [0x10-10] 固定記憶體位址：存取 Memory-Mapped 資料

## 原始來源

- [A 'C' Test: The 0x10 Best Questions for Would-be Embedded Programmers — Accessing Fixed Memory Locations](https://rmbconsulting.us/publications/a-c-test-the-0x10-best-questions-for-would-be-embedded-programmers/#accessing-fixed-memory-locations)

## 題目

某個嵌入式專案要求程式把絕對位址 `0x67a9` 的整數位置設為 `0xaa55`，而且編譯器是純 ANSI C。要如何撰寫？

原題期待的核心操作是：把整數形式的位址轉成指標，再 dereference 指標完成寫入。

## 先說結論／面試回答

若平台文件已保證該位址有效、對齊方式正確，而且那裡確實是一個可寫的 `int`，最直接的語法是：

```c
int *ptr = (int *)0x67a9;
*ptr = 0xaa55;
```

但在實際韌體面試中，我不會停在這兩行。這個舊題把許多平台條件藏起來了：

- 整數轉指標的結果由實作定義，不是跨所有 ANSI C 平台都可攜的位址機制。
- `0x67a9` 是奇數位址，對 2-byte 或 4-byte `int` 很可能未對齊。
- `int` 寬度不固定；寫入可能是 16 或 32 bits。
- Memory-mapped register 通常需要 `volatile`，否則編譯器可能合併、移除或重排不具一般記憶體語意的存取。
- 位址是否 mapped、可寫，以及寫入是否有副作用，只能由 datasheet、linker script 與 memory map 決定。

在有明確 16-bit register 規格的真實專案中，會更接近：

```c
#include <stdint.h>

#define DEVICE_DATA_ADDRESS UINT32_C(0x40001000)
#define DEVICE_DATA \
    (*(volatile uint16_t *)(uintptr_t)DEVICE_DATA_ADDRESS)

DEVICE_DATA = UINT16_C(0xAA55);
```

這仍然是平台相依程式碼；`uintptr_t` 讓「可容納轉換後指標值的無號整數型別」更明確，但不會憑空讓任意數字變成有效硬體位址。`uintptr_t` 是 `<stdint.h>` 的 optional typedef，只有實作提供能和 `void *` 往返轉換的無號整數型別時才會定義；實際韌體仍要以工具鏈與 BSP 的介面為準。

## 觀念拆解

### Cast 只改變 C 如何解讀數值

```c
(int *)0x67a9
```

告訴編譯器把整數常值轉成 `int *`。它不會配置記憶體、不會詢問作業系統，也不會驗證地址上真的存在一個 `int`。接著的 `*ptr = value` 才會產生寫入動作。

因此「能通過編譯」和「能安全執行」是兩件完全不同的事。在一般 Windows process 中，低位址通常沒有映射，真的解參考很可能觸發 access violation；在 MCU 上，相同數字則可能是 Flash、RAM、peripheral、保留區或根本不存在。

### 為什麼需要 `volatile`

Hardware register 的值可能被周邊改變，寫入也可能觸發硬體動作。`volatile` 要求抽象機保留這些 observable accesses，避免把看似「沒有一般程式讀取」的 write 當成無用寫入移除。

```c
volatile uint16_t *reg;
```

這裡是「指向 volatile `uint16_t` 的指標」。若還希望指標本身初始化後不可改指向，可以寫：

```c
volatile uint16_t * const reg = /* platform address */;
```

`volatile` 不會驗證位址、不保證原子性，也不取代 memory barrier 或 device-specific ordering primitive。

### 寬度、對齊與 endian

寫 `int` 會使用 `sizeof(int)` 的寬度。本題只給 16-bit 值 `0xAA55`，卻沒有說 register 寬度；若 `int` 是 32 bits，實際 bus transaction 可能多寫兩個 bytes。

同時，`0x67a9` 對 2-byte、4-byte 對齊都不自然。有些 CPU 支援未對齊但較慢，有些會 fault，有些 peripheral bus 根本不允許。即使成功寫入，little-endian 與 big-endian 系統在 byte address 上看到的 `AA`、`55` 順序也不同。

## 自我實作

我不在 Windows 上解參考 `0x67a9`。安全範例用結構模擬兩個相鄰的 16-bit hardware registers，再透過 `volatile uint16_t * const` 寫入第一個欄位。完整程式放在同目錄的 `example.c`：

```c
#include <stdint.h>
#include <stdio.h>

struct simulated_peripheral {
    volatile uint16_t data;
    volatile uint16_t neighbor;
};

int main(void)
{
    struct simulated_peripheral device = {
        UINT16_C(0x0000),
        UINT16_C(0x1234)
    };
    volatile uint16_t * const register_pointer = &device.data;

    printf("simulated register width: %lu bits\n",
           (unsigned long)(sizeof *register_pointer * 8U));
    printf("before write: 0x%04lX\n",
           (unsigned long)*register_pointer);

    *register_pointer = UINT16_C(0xAA55);

    printf("after write : 0x%04lX\n",
           (unsigned long)*register_pointer);
    printf("neighbor    : 0x%04lX\n",
           (unsigned long)device.neighbor);

    return 0;
}
```

這個實驗保留了「透過 volatile 指標寫指定 register」的核心，但把地址換成 C 確實建立、有效且對齊的物件，避免用桌面 OS 測試不存在的硬體。

## 編譯與執行指令

測試環境是 Windows 上的 **32-bit MinGW GCC 6.3.0 編譯目標**（`gcc -dumpmachine` 為 `mingw32`）：

```powershell
gcc -std=c11 -Wall -Wextra -pedantic example.c -o example.exe
.\example.exe
```

## 本機實際輸出

```text
simulated register width: 16 bits
before write: 0x0000
after write : 0xAA55
neighbor    : 0x1234
```

編譯器沒有產生 warning。

## 結果理解

`sizeof *register_pointer` 證明模擬 register 的寬度是 16 bits。寫入前讀到 `0x0000`，執行：

```c
*register_pointer = UINT16_C(0xAA55);
```

後讀回 `0xAA55`，代表指標確實指向 `device.data`。相鄰的 `neighbor` 仍是 `0x1234`，也證明這次使用 16-bit 型別時沒有覆蓋下一個 register。

這個結果只能驗證 C 指標存取與資料寬度；它不等於驗證任何真實 MCU 的 bus、register side effect 或絕對位址。

## 嵌入式／平台注意事項

- 先查 reference manual 的 register 位址、access width、對齊、read/write 權限與 reset value。
- 有些 register 是 read-only、write-only、write-one-to-clear 或 read-to-clear；不能一律用一般變數的 read-modify-write 思考。
- 使用 vendor CMSIS／HAL register definition 通常比在應用程式內重複手寫裸位址安全，因為型別、offset 與保留欄位已有統一定義。
- `volatile` 只處理編譯器層級的 access，不保證 CPU／bus ordering。DMA、多核心或 cache coherent 問題可能還需要 barrier 與 cache maintenance。
- 若 linker script 把一般物件放到絕對位置，還要確認 startup code 是否初始化它，以及該區域是否和 peripheral address 重疊。
- 在 hosted OS user space 直接碰實體位址通常不合法；需透過 driver、memory mapping API 或 kernel interface。

## 小結

原題的語法重點是「整數位址轉指標，再 dereference」。工程實務的完整答案還必須補上 `volatile`、固定寬度型別、合法 mapping、alignment、endianness 與 register side effect。Cast 能讓程式通過型別檢查，但不能讓錯誤位址變安全。

## 參考資料

- [RMB Consulting：原始第 10 題](https://rmbconsulting.us/publications/a-c-test-the-0x10-best-questions-for-would-be-embedded-programmers/#accessing-fixed-memory-locations)
- [Medium：工程師應知道的 0x10 個問題（10）— 存取固定的記憶體位置](https://medium.com/@racktar7743/%E5%B7%A5%E7%A8%8B%E5%B8%AB%E6%87%89%E7%9F%A5%E9%81%93%E7%9A%840x10%E5%80%8B%E5%95%8F%E9%A1%8C-10-%E5%AD%98%E5%8F%96%E5%9B%BA%E5%AE%9A%E7%9A%84%E8%A8%98%E6%86%B6%E9%AB%94%E4%BD%8D%E7%BD%AE-f48fc960e70c)
- [ISO C11 working draft N1570，6.3.2.3 Pointers](https://www.open-std.org/jtc1/sc22/wg14/www/docs/n1570.pdf)
