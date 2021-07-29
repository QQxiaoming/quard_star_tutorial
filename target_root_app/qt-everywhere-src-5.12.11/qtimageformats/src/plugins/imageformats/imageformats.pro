TEMPLATE = subdirs
SUBDIRS = \
#    dds \
    tga \
    wbmp

include($$OUT_PWD/../../imageformats/qtimageformats-config.pri)
QT_FOR_CONFIG += imageformats-private

qtConfig(tiff): SUBDIRS += tiff
qtConfig(webp): SUBDIRS += webp

darwin: SUBDIRS += macheif

qtConfig(regularexpression): \
    SUBDIRS += icns

qtConfig(mng): SUBDIRS += mng
qtConfig(jasper) {
    SUBDIRS += jp2
} else:darwin: {
    SUBDIRS += macjp2
}

winrt {
    SUBDIRS -= tiff \
               tga \
               webp
}
