TEMPLATE = app

QT += quick qml
SOURCES += main.cpp

content.prefix = /demos/tweetsearch
content.files = \
        tweetsearch.qml \
        content/FlipBar.qml \
        content/LineInput.qml \
        content/ListFooter.qml \
        content/ListHeader.qml \
        content/SearchDelegate.qml \
        content/TweetDelegate.qml \
        content/tweetsearch.mjs \
        content/TweetsModel.qml \
        content/resources/anonymous.png \
        content/resources/bird-anim-sprites.png \
        content/resources/icon-clear.png \
        content/resources/icon-loading.png \
        content/resources/icon-refresh.png \
        content/resources/icon-search.png

RESOURCES += content

OTHER_FILES = tweetsearch.qml \
              content/*.qml \
              content/*.mjs \
              content/resources/*

target.path = $$[QT_INSTALL_EXAMPLES]/demos/tweetsearch
INSTALLS += target
