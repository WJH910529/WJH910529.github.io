# WJH Blog 更新指南

這份文件是給之後更新 `WJH910529.github.io` 用的。

你的 blog 原始碼放在 GitHub 的 `main` 分支，真正顯示在網站上的靜態網頁會部署到 `gh-pages` 分支。

網站網址：

```text
https://WJH910529.github.io/
```

GitHub repository：

```text
git@github.com:WJH910529/WJH910529.github.io.git
```

## 重要觀念

你平常修改的是 Markdown 原始文章，不是直接修改網站 HTML。

流程是：

```text
修改 Markdown -> commit -> push 到 main -> deploy 到 gh-pages -> GitHub Pages 更新網站
```

兩個分支的用途：

- `main`：保存你的部落格原始碼，例如 Markdown、設定檔、package.json。
- `gh-pages`：保存 VuePress 編譯後的靜態網站，GitHub Pages 會讀這個分支來顯示網站。

所以建議每次更新文章後都做兩件事：

1. 把原始碼推到 `main`
2. 執行部署，把網站更新到 `gh-pages`

## 文章放在哪裡

主要文章都放在：

```text
src/
```

目前常用資料夾：

```text
src/README.md                         首頁
src/about/README.md                   關於我
src/notes/                            筆記文章
src/learning-log/                     學習紀錄
src/projects/                         專案介紹
src/en/                               英文版頁面
```

繁體中文文章通常放在：

```text
src/notes/
src/learning-log/
src/projects/
```

英文文章通常放在：

```text
src/en/notes/
src/en/learning-log/
src/en/projects/
```

## 新增一篇中文文章

建議每篇文章都用一個資料夾，裡面放 `README.md`。

例如要新增一篇文章：

```text
src/notes/my-new-note/README.md
```

內容範例：

~~~md
---
title: 我的新文章
date: 2026-07-07
category:
  - Notes
tag:
  - Example
  - Markdown
---

# 我的新文章

這裡開始寫文章內容。

## 小標題

- 第一點
- 第二點
- 第三點

```c
#include <stdio.h>

int main(void) {
  printf("Hello, blog!\n");
  return 0;
}
```
~~~

注意：上面的最前面 `---` 區塊叫 frontmatter，用來設定文章標題、日期、分類、標籤。

## 讓新文章出現在側邊欄

如果你新增的是 `src/notes/my-new-note/README.md`，還要更新側邊欄設定：

```text
src/.vuepress/sidebars/zh.ts
```

找到 `Notes` 的 `children`，加入你的文章資料夾：

```ts
{
  text: "Notes",
  collapsible: true,
  prefix: "/notes/",
  children: [
    "",
    "first-note/",
    "markdown-demo/",
    "my-new-note/",
  ],
},
```

如果你新增在 `learning-log` 或 `projects`，就更新同一個檔案裡對應區塊的 `children`。

## 新增英文文章

英文版文章放在：

```text
src/en/notes/
```

例如：

```text
src/en/notes/my-new-note/README.md
```

然後更新英文側邊欄：

```text
src/.vuepress/sidebars/en.ts
```

加入：

```ts
"my-new-note/",
```

如果沒有要維護英文版，也可以先只更新中文文章。

## 修改首頁、關於我、專案頁

首頁：

```text
src/README.md
```

關於我：

```text
src/about/README.md
```

專案總覽：

```text
src/projects/README.md
```

WJH Blog 專案頁：

```text
src/projects/wjh-blog/README.md
```

英文版對應頁面在：

```text
src/en/
```

## 圖片怎麼放

如果文章需要圖片，可以把圖片放在文章同一個資料夾內。

例如：

```text
src/notes/my-new-note/README.md
src/notes/my-new-note/demo.png
```

Markdown 內這樣引用：

```md
![示範圖片](./demo.png)
```

如果是很多頁都會共用的圖片，可以放在：

```text
src/.vuepress/public/
```

例如：

```text
src/.vuepress/public/images/avatar.png
```

Markdown 內這樣引用：

```md
![Avatar](/images/avatar.png)
```

## 每次更新文章後的標準流程

修改完文章後，進入 `wjh-blog` 資料夾執行：

```powershell
git status
```

確認有看到你修改的檔案後，加入 Git：

```powershell
git add -A
```

建立 commit：

```powershell
git commit -m "Update blog"
```

推送原始碼到 GitHub 的 `main`：

```powershell
git push
```

部署網站到 GitHub Pages：

```powershell
corepack pnpm run deploy
```

看到以下訊息代表部署成功：

```text
Published
```

部署完成後，到這裡看網站：

```text
https://WJH910529.github.io/
```

GitHub Pages 有時候需要等幾分鐘才會更新。

## 最短更新流程

如果你已經很熟，可以照這組指令：

```powershell
git status
git add -A
git commit -m "Update blog"
git push
corepack pnpm run deploy
```

## 避免電腦變卡

平常只是修改文章時，不需要執行：

```powershell
corepack pnpm run dev
```

`dev` 是本機預覽用的開發伺服器，會持續監看檔案變更，所以它會常駐並吃一些電腦資源。

只有在你想要發布前先用本機網址預覽網站時，才需要執行：

```powershell
corepack pnpm run dev
```

用完後，在終端機按：

```text
Ctrl + C
```

就可以停止本機開發伺服器。

如果只是正常更新文章並發布到 GitHub Pages，照這組就好：

```powershell
git add -A
git commit -m "Update blog"
git push
corepack pnpm run deploy
```

## 如果沒有東西可以 commit

如果執行：

```powershell
git commit -m "Update blog"
```

看到類似：

```text
nothing to commit
```

代表目前沒有新的原始碼變更。

如果你只是想重新發布目前版本，可以直接執行：

```powershell
corepack pnpm run deploy
```

## Build 警告可以先忽略

部署時你可能會看到：

```text
[INVALID_ANNOTATION]
[PLUGIN_TIMINGS]
```

這些目前是依賴套件和打包器的警告，不是你的文章錯誤。

只要最後有看到：

```text
success VuePress build completed
Published
```

就代表網站部署成功。

## 搜尋、深色模式、語言切換

目前網站已恢復 VuePress Theme Hope 的工具列功能：

- 搜尋文章：`Ctrl + K`
- 深色/淺色模式切換
- 繁體中文 / English 語言切換

搜尋功能使用本地搜尋套件：

```text
@vuepress/plugin-slimsearch
```

新增文章並部署後，搜尋索引也會一起更新。

## 常見錯誤

### 忘記 push

如果只執行：

```powershell
corepack pnpm run deploy
```

網站可能會更新，但你的 Markdown 原始碼沒有推到 GitHub 的 `main`。

建議每次都先：

```powershell
git add -A
git commit -m "Update blog"
git push
```

再部署。

### 忘記更新 sidebar

如果文章存在，但側邊欄沒有出現，通常是忘記更新：

```text
src/.vuepress/sidebars/zh.ts
```

或英文版：

```text
src/.vuepress/sidebars/en.ts
```

### GitHub Pages 還沒更新

如果部署成功但網站看起來還是舊的，等幾分鐘再重新整理。

也可以嘗試強制重新整理瀏覽器：

```text
Ctrl + F5
```

## 不要手動改這些資料夾

不要手動修改：

```text
src/.vuepress/dist/
node_modules/
```

原因：

- `src/.vuepress/dist/` 是部署時自動產生的網站成品。
- `node_modules/` 是套件安裝資料夾。

平常只需要改：

```text
src/
src/.vuepress/sidebars/
src/.vuepress/config.ts
src/.vuepress/theme.ts
```

其中最常改的就是 `src/` 裡面的 Markdown 文章。
