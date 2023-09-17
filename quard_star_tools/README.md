# Quard Star Tools

Quard Star Tools是Quard Star Tutorial项目的一个子项目。其基于Qt编写的Quard Star Tutorial项目的GUI前端可视化工具，主要目的是直观的展示，以及初学者新手使用。所有依赖已经集成在内部，不需要额外添加。请基于Qt 6.2.0及以上版本编译运行。

## 项目依赖

本项目中引用了部分其他项目的开源代码，全部遵守原项目相关许可协议，并做了些适配修改：

- qvncclientwidget.cpp fork自https://bitbucket.org/amahta/qvncclient.git
- QTelnet.cpp fork自https://github.com/silderan/QTelnet.git
- qtermwidget fork自https://github.com/lxqt/qtermwidget （去除了pty部分的实现，因为原项目不支持windows平台，本项目不需要pty部分因此去除后可跨平台使用。）
- QFontIcon fork自https://github.com/dridk/QFontIcon
- lwext4 fork自https://github.com/gkostka/lwext4
- ff15 fork自http://elm-chan.org/fsw/ff/arc/ff15.zip
- jffs2 fork自https://github.com/rickardp/jffs2extract
- treemodel.cpp fork自https://github.com/chocoball/QTreeViewTest
- QDarkStyleSheet fork自https://github.com/ColinDuquesnoy/QDarkStyleSheet

本项目中最初查看分析文件系统映像文件的需求编写了qfsviewer.cpp等相关代码，后考虑该功能的通用性需求，将其拆分为单独的项目发布，详见https://github.com/QQxiaoming/QFSViewer.git，未来将该项目作为本项目的上游依赖同步更新。
