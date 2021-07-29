defineTest(qtConfLibrary_fbx) {
    libdir =

    prefix = $$getenv(FBXSDK)
    !isEmpty(prefix) {
        !qtConfResolvePathIncs($${1}.includedir, $${prefix}/include, $$2): \
            return(false)
        !win32: libdir += $${prefix}/lib
    }

    libs = $$getenv(FBXSDK_LIBS)
    isEmpty(libs): \
        libs = $$eval($${1}.libs)

    !qtConfResolvePathLibs($${1}.libs, $$libdir, $$libs): \
        return(false)

    return(true)
}

