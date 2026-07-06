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
