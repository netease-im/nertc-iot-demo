import { defineUserConfig } from "vuepress";

import theme from "./theme.js";

export default defineUserConfig({
  base: "/projects/xiaozhi/",

  locales: {
    "/en/": {
      lang: "en-US",
      title: "Xiaozhi Encyclopedia",
      description: "Xiaozhi Encyclopedia based on SF32",
    },
    "/": {
      lang: "zh-CN",
      title: "小智百科全书",
      description: "基于SF32的小智百科全书",
    },
  },

  theme,

  // Enable it with pwa
  // shouldPrefetch: false,
});
