import { sidebar } from "vuepress-theme-hope";

export default sidebar([
  "/en/",
  "/en/about/",
  {
    text: "Notes",
    collapsible: true,
    prefix: "/en/notes/",
    children: [
      "",
      "first-note/",
    ],
  },
  {
    text: "Learning Log",
    collapsible: true,
    prefix: "/en/learning-log/",
    children: [
      "",
    ],
  },
  {
    text: "Projects",
    collapsible: true,
    prefix: "/en/projects/",
    children: [
      "",
    ],
  },
]);
