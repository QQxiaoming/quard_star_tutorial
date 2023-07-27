# Quard Star Tools

Quard Star Tools是Quard Star Tutorial项目的一个子项目。其基于Qt编写的Quard Star Tutorial项目的GUI前端可视化工具，主要目的是直观的展示，以及初学者新手使用。

## 项目依赖

本项目中引用了部分其他项目的开源代码，全部遵守原项目相关许可协议，并做了些适配修改：

- qvncclientwidget.cpp fork自https://github.com/anzyelay/qvncclient.git
- QTelnet.cpp fork自https://github.com/silderan/QTelnet.git
- qtermwidget fork自https://github.com/lxqt/qtermwidget （去除了pty部分的实现，因为原项目不支持windows平台，本项目不需要pty部分因此去除后可跨平台使用。）
- QFontIcon fork自https://github.com/dridk/QFontIcon
- ext4 fork自https://github.com/ggetchev/pyext4
- ff15 fork自http://elm-chan.org/fsw/ff/arc/ff15.zip
- jffs2 fork自https://github.com/rickardp/jffs2extract
- treemodel.cpp fork自https://github.com/chocoball/QTreeViewTest
