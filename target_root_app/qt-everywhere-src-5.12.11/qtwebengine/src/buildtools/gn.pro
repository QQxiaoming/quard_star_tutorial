TEMPLATE = aux
option(host_build)

!debug_and_release: CONFIG += release

include($$QTWEBENGINE_OUT_ROOT/src/core/qtwebenginecore-config.pri)
QT_FOR_CONFIG += webenginecore-private

build_pass|!debug_and_release {
    !qtConfig(webengine-system-gn): CONFIG(release, debug|release) {
        buildgn.target = build_gn
        out = $$gnPath()
        out_path = $$dirname(out)
        !qtConfig(webengine-system-ninja): ninja_path = $$ninjaPath()
        else: ninja_path="ninja"
        # check if it is not already build
        !exists($$out) {
            src_3rd_party_dir = $$absolute_path("$${getChromiumSrcDir()}/../", "$$QTWEBENGINE_ROOT")
            gn_bootstrap = $$system_path($$absolute_path(gn/build/gen.py, $$src_3rd_party_dir))

            gn_configure = $$system_quote($$gn_bootstrap) --no-last-commit-position --out-path $$out_path
            !system("$$pythonPathForSystem() $$gn_configure") {
                error("GN generation error!")
            }
            !system("cd $$system_quote($$system_path($$out_path)) && $$ninja_path $$basename(out)" ) {
                error("GN build error!")
            }
        }
        QMAKE_DISTCLEAN += $$out
    }
}
