TEMPLATE = subdirs

SUBDIRS += webview imports plugins
plugins.depends = webview
imports.depends = webview

android: SUBDIRS += jar
