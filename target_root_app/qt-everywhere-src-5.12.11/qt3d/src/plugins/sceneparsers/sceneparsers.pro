TEMPLATE = subdirs
# QNX is not supported, and Linux GCC 4.9 on ARM chokes on the assimp
# sources (https://gcc.gnu.org/bugzilla/show_bug.cgi?id=66964).
QT_FOR_CONFIG += 3dcore-private
!ios:!tvos:!qcc:qtConfig(assimp):if(qtConfig(system-assimp)|android-clang|clang|win32-msvc)|if(gcc:greaterThan(QT_GCC_MAJOR_VERSION, 4)) {
    SUBDIRS += assimp
}
SUBDIRS += gltf

qtConfig(temporaryfile):qtConfig(regularexpression) {
    SUBDIRS += gltfexport
}
