// ==UserScript==
// @name           Test bad match script
// @homepageURL    http://www.qt.io/
// @description    Test script with metadata block with an invalid match directive
// @match          some:junk
// @run-at         document-end
// ==/UserScript==

document.title = "New title for some:junk";
