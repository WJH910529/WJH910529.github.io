import { hopeTheme } from "vuepress-theme-hope";
import zhSidebar from "./sidebars/zh.js";
import enSidebar from "./sidebars/en.js";

const footer = "WJH Blog - built with VuePress Theme Hope.";

export default hopeTheme({
  hostname: "https://WJH910529.github.io",
  contributors: false,
  darkmode: "switch",

  author: {
    name: "WJH",
    url: "https://WJH910529.github.io",
  },

  repo: "WJH910529/WJH910529.github.io",
  repoDisplay: false,
  docsDir: "src",
  displayFooter: true,

  locales: {
    "/": {
      lang: "zh-TW",
      navbarLocales: {
        langName: "繁體中文",
        selectLangAriaLabel: "選擇語言",
      },
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
    "/en/": {
      lang: "en-US",
      navbarLocales: {
        langName: "English",
      },
      navbar: [
        "/en/",
        "/en/about/",
        "/en/notes/",
        "/en/learning-log/",
        "/en/projects/",
      ],
      sidebar: enSidebar,
      footer,
      blog: {
        intro: "/en/about/",
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
    slimsearch: {
      indexContent: true,
    },
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
