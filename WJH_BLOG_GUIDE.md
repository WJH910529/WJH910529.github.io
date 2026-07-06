# WJH Blog Guide

This folder is now your `WJH Blog` VuePress project.

The previous article content has been removed. The site now keeps only a small set of example pages so you can copy their structure later.

## Local Preview

Run:

```powershell
cd C:\Users\user\Desktop\WJH\wjh-blog
corepack pnpm run dev
```

Then open:

```text
http://localhost:8080/
```

## Build Static Files

Run:

```powershell
cd C:\Users\user\Desktop\WJH\wjh-blog
corepack pnpm run build:vite
```

The static output is generated here:

```text
src/.vuepress/dist
```

That `dist` folder is what GitHub Pages ultimately serves as a website.

## Current Content Structure

```text
src/README.md
src/about/README.md
src/notes/README.md
src/notes/first-note/README.md
src/notes/markdown-demo/README.md
src/learning-log/README.md
src/learning-log/2026-07-06-start/README.md
src/projects/README.md
src/projects/wjh-blog/README.md
```

## Important Config Files

```text
package.json
src/.vuepress/config.ts
src/.vuepress/theme.ts
src/.vuepress/sidebar.ts
src/.vuepress/sidebars/zh.ts
```

Main roles:

- `package.json`: commands for dev, build, deploy.
- `src/README.md`: home page.
- `src/.vuepress/config.ts`: VuePress site title, description, base path.
- `src/.vuepress/theme.ts`: Theme Hope settings.
- `src/.vuepress/sidebars/zh.ts`: sidebar entries.

## Add A New Article

Example path:

```text
src/notes/my-new-note/README.md
```

Example content:

```markdown
---
title: My New Note
date: 2026-07-06
category:
  - Notes
tag:
  - example
---

# My New Note

Write your article here.
```

Then add it to:

```text
src/.vuepress/sidebars/zh.ts
```

For example:

```ts
children: [
  "",
  "first-note/",
  "markdown-demo/",
  "my-new-note/",
],
```

## GitHub Pages

After the site is deployed to GitHub Pages, you do not need to keep your computer on.

GitHub hosts the generated static files. Other people can visit your website anytime through GitHub's server.

For a personal GitHub Pages site, create a repository named:

```text
WJH910529.github.io
```

Then the website URL will be:

```text
https://WJH910529.github.io/
```

Because this is a personal site, keep this setting:

```ts
base: "/",
```

## Git Remote

This project was created from a template repository, so the template remote is kept as:

```text
template-source
```

When you create your own GitHub repository, add it as `origin`:

```powershell
git remote add origin git@github.com:WJH910529/WJH910529.github.io.git
```

If you deploy to a normal project repository like `wjh-blog`, then the URL is usually:

```text
https://WJH910529.github.io/wjh-blog/
```

In that case, change `base` to:

```ts
base: "/wjh-blog/",
```

## Deploy Script

`package.json` currently uses a placeholder:

```json
"deploy": "pnpm run build:vite && gh-pages -d ./src/.vuepress/dist --nojekyll -r git@github.com:WJH910529/WJH910529.github.io.git"
```

The deploy script now points to your `WJH910529.github.io` repository.
