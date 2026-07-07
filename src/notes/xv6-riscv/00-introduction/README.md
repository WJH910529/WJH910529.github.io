---
title: 00. 開始學習 xv6-riscv
date: 2026-07-08
category:
  - Notes
tag:
  - xv6
  - riscv
  - operating-system
---

# 00. 開始學習 xv6-riscv

這篇是 xv6-riscv 學習系列的起點。

我想透過 xv6-riscv 重新整理作業系統的基礎觀念，並把閱讀原始碼、完成 lab、遇到的問題與解法記錄下來。

## 為什麼學 xv6

xv6 的程式碼規模相對小，卻保留了作業系統的重要概念。比起只看課本，直接閱讀 xv6 的程式碼可以更具體地理解核心功能是怎麼實作出來的。

## 目前想理解的問題

- 作業系統是怎麼從開機流程進入 kernel 的？
- process 是怎麼被建立、切換與排程的？
- virtual memory 和 page table 實際上怎麼運作？
- system call 如何從 user mode 進入 kernel mode？
- file system 如何把資料組織在磁碟上？

## 後續筆記格式

之後每篇筆記大概會用這個格式：

```text
主題
我想解決的問題
閱讀到的程式碼
關鍵觀念
實作或 lab 紀錄
遇到的錯誤與解法
心得
```

## 學習參考資料

我先把 xv6-riscv 的參考資料分成幾類：官方原始資料、書籍與翻譯、Lab / 實作參考、前人學習整理。之後讀到卡住時，可以依照問題類型回來找對應的資料。

### MIT 官方原始資料

- [mit-pdos/xv6-riscv](https://github.com/mit-pdos/xv6-riscv)
  - xv6-riscv 的官方原始碼倉庫。
  - 之後如果要看 kernel、user program、build script，會以這裡的程式碼為主要依據。

- [MIT 6.S081 / 6.1810 2022 Schedule](https://pdos.csail.mit.edu/6.S081/2022/schedule.html)
  - MIT 作業系統課程的官方課表。
  - 可以用來對照每週主題、閱讀章節、lecture note 和 lab 順序。

### 書籍與翻譯

- [xv6: a simple, Unix-like teaching operating system](https://app.notion.com/p/xv6-a-simple-Unix-like-teaching-operating-system-30d6b43b837a80fea9dfe66baca52101?pvs=21)
  - xv6 book 相關的閱讀資料或整理頁。
  - 可以當作閱讀 xv6 book 時的輔助索引。

- [xv6 中文文檔](https://th0ar.gitbooks.io/xv6-chinese/content/index.html)
  - xv6 的中文翻譯資料。
  - 適合在英文原文不好理解時，先抓整體概念。

- [Mes0903: xv6-riscv book zh-TW](https://mes0903.github.io/OS/xv6-riscv-book-zh-TW/chapter1/)
  - xv6-riscv book 的繁體中文參考資料。
  - 可以用來輔助閱讀 xv6-riscv 版本的章節內容。

### Lab / 實作參考

- [Iuriak/OS-Xv6-Lab-2023](https://github.com/Iuriak/OS-Xv6-Lab-2023)
  - 同濟大學作業系統 xv6 lab 專案。
  - 可以作為 lab 題目、實作方向與專案整理方式的參考。

### 前人做過的學習整理

- [6.1810 Operation System Engineering Overview](https://medium.com/@igimast5088/6-1810-operation-system-engineering-overview-86a1a8c2948d)
  - MIT 6.1810 的課程與 xv6 學習總覽。
  - 適合在開始前先看整體路線。

- [Yama's Trail: xv6 學習紀錄](https://yamatrail.com/xv6/)
  - 依照 MIT 6.1810 lab 與主題整理的學習紀錄。
  - 適合用來對照自己每個 lab 的進度。

- [CSDN: MIT 6.S081 / 6.1810 Lab 筆記](https://blog.csdn.net/J__M__C/article/details/131908419)
  - 中文 lab 筆記與實作紀錄。
  - 適合在卡住時參考思路，但不應該直接複製答案。

- [LRL52: OS 分類文章](https://lrl52.top/category/os/)
  - 作業系統相關文章分類。
  - 可以用來補充不同作者對 OS / xv6 主題的整理方式。

## 我的使用方式

- 先以 MIT 官方 repo 和課程 schedule 當主線。
- 閱讀 xv6 book 時，用中文翻譯輔助理解。
- 實作 lab 時先自己嘗試，真的卡住再看前人的筆記。
- 參考別人的解法時，重點放在理解為什麼這樣改，而不是直接抄程式碼。

## 下一步

接下來會先整理 xv6-riscv 的環境、專案結構與啟動流程。
