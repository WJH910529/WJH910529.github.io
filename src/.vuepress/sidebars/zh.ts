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
          "lab1-xv6-and-unix-utilities/",
          "lab2-system-calls/",
        ],
      },
      {
        text: "0x10 韌體面試問題集",
        collapsible: true,
        prefix: "0x10-firmware-interview/",
        children: [
          "",
          "01-seconds-per-year/",
          "02-min-macro/",
          "03-error-directive/",
          "04-infinite-loop/",
          "05-c-declarations/",
          "06-static/",
          "07-const/",
          "08-volatile/",
          "09-bit-manipulation/",
          "10-fixed-memory-address/",
          "11-interrupt-service-routine/",
          "12-signed-unsigned/",
          "13-word-length-complement/",
          "14-dynamic-memory-allocation/",
          "15-typedef-vs-define/",
          "16-maximum-munch/",
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
