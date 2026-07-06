import { defineUserConfig } from "vuepress";

import theme from "./theme.js";

export default defineUserConfig({
  base: "/",

  locales: {
    "/": {
      lang: "zh-TW",
      title: "WJH Blog",
      description: "Notes, projects, and learning records.",
    },
  },

  theme,

  // Enable it with pwa
  // shouldPrefetch: false,
});
