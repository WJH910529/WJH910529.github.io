import { sidebar } from "vuepress-theme-hope";

export default sidebar([
  "/",
  "/about/",
  {
    text: "Notes",
    collapsible: true,
    prefix: "/notes/",
    children: [
      "",
      {
        text: "xv6-riscv",
        collapsible: true,
        prefix: "xv6-riscv/",
        children: [
          "",
          "00-introduction/",
        ],
      },
      "first-note/",
      "markdown-demo/",
    ],
  },
  {
    text: "Learning Log",
    collapsible: true,
    prefix: "/learning-log/",
    children: [
      "",
      "2026-07-06-start/",
    ],
  },
  {
    text: "Projects",
    collapsible: true,
    prefix: "/projects/",
    children: [
      "",
      "wjh-blog/",
    ],
  },
]);
