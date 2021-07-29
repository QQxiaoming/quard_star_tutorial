requires(contains(QT_CONFIG, opengl))
requires(qtHaveModule(network))

load(qt_parts)

# We need opengl, minimum es2
contains(QT_CONFIG, opengles1) {
    error("QtCanvas3D does not support OpenGL ES 1!")
}
