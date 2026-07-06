import { hopeTheme } from "vuepress-theme-hope";
import zhSidebar from "./sidebars/zh.js";

const footer = "WJH Blog - built with VuePress Theme Hope.";

export default hopeTheme({
  hostname: "https://WJH910529.github.io",
  contributors: false,

  author: {
    name: "WJH",
    url: "https://WJH910529.github.io",
  },

  repo: "",
  repoDisplay: false,
  docsDir: "src",
  displayFooter: true,

  locales: {
    "/": {
      lang: "zh-TW",
      navbar: [
        "/",
        "/about/",
        "/notes/",
        "/learning-log/",
        "/projects/",
      ],
      sidebar: zhSidebar,
      footer,
      blog: {
        intro: "/about/",
        description: "Notes, projects, and learning records.",
      },
    },
  },

  markdown: {
    align: true,
    attrs: false,
    codeTabs: true,
    component: true,
    figure: true,
    gfm: true,
    imgLazyload: true,
    imgSize: true,
    mark: true,
    sub: true,
    sup: true,
    tabs: true,
    tasklist: true,
    vPre: true,
    highlighter: {
      type: "shiki",
      highlightLines: true,
      lineNumbers: true,
    },
  },

  plugins: {
    blog: {
      excerptLength: 0,
    },
    catalog: false,
    git: {
      createdTime: false,
      updatedTime: false,
      contributors: false,
      changelog: false,
    },
    components: {
      components: ["Badge", "VPCard"],
    },
    icon: {
      prefix: "fa6-solid:",
    },
    feed: {
      rss: true,
      atom: true,
      json: true,
    },
  },
});
