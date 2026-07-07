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

## 下一步

接下來會先整理 xv6-riscv 的環境、專案結構與啟動流程。
