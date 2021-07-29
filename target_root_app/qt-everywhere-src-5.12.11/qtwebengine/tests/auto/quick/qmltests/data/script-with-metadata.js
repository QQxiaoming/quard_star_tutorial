// ==UserScript==
// @name           Test script
// @homepageURL    http://www.qt.io/
// @description    Test script with metadata block
// @include        *data/test*.html
// @include        /favicon.html?$/
// @exclude        *test2.html
// @exclude        /test[-]iframe/
// @run-at         document-end
// ==/UserScript==

document.title = "New title";
