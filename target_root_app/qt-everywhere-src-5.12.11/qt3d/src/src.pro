TEMPLATE = subdirs

!qtHaveModule(gui): \
    return()

src_core.subdir = $$PWD/core
src_core.target = sub-core

src_render.subdir = $$PWD/render
src_render.target = sub-render
src_render.depends = src_core

src_logic.subdir = $$PWD/logic
src_logic.target = sub-logic
src_logic.depends = src_core

src_input.subdir = $$PWD/input
src_input.target = sub-input
src_input.depends = src_core

src_animation.subdir = $$PWD/animation
src_animation.target = sub-animation
src_animation.depends = src_render

src_extras.subdir = $$PWD/extras
src_extras.target = src_extras
src_extras.depends = src_render src_input src_logic

src_doc.subdir = $$PWD/doc
src_doc.target = sub-doc
src_doc.depends = src_animation src_input

qtHaveModule(quick) {
    # Quick3D libs
    src_quick3d_core.subdir = $$PWD/quick3d/quick3d
    src_quick3d_core.target = sub-quick3d-core
    src_quick3d_core.depends = src_core

    src_quick3d_render.subdir = $$PWD/quick3d/quick3drender
    src_quick3d_render.target = sub-quick3d-render
    src_quick3d_render.depends = src_render src_quick3d_core

    src_quick3d_input.subdir = $$PWD/quick3d/quick3dinput
    src_quick3d_input.target = sub-quick3d-input
    src_quick3d_input.depends = src_input src_quick3d_core

    src_quick3d_animation.subdir = $$PWD/quick3d/quick3danimation
    src_quick3d_animation.target = sub-quick3d-animation
    src_quick3d_animation.depends = src_animation src_quick3d_core src_quick3d_render

    src_quick3d_extras.subdir = $$PWD/quick3d/quick3dextras
    src_quick3d_extras.target = sub-quick3d-extras
    src_quick3d_extras.depends = src_render src_logic src_input src_extras src_quick3d_core

    src_quick3d_scene2d.subdir = $$PWD/quick3d/quick3dscene2d
    src_quick3d_scene2d.target = sub-quick3d-scene2d
    src_quick3d_scene2d.depends = src_render src_logic src_input src_quick3d_core

    # Quick3D imports
    src_quick3d_core_imports.file = $$PWD/quick3d/imports/core/importscore.pro
    src_quick3d_core_imports.target = sub-quick3d-imports-core
    src_quick3d_core_imports.depends = src_quick3d_core

    src_quick3d_imports_render.file = $$PWD/quick3d/imports/render/importsrender.pro
    src_quick3d_imports_render.target = sub-quick3d-imports-render
    src_quick3d_imports_render.depends = src_quick3d_render

    src_quick3d_imports_scene3d.file = $$PWD/quick3d/imports/scene3d/importsscene3d.pro
    src_quick3d_imports_scene3d.target = sub-quick3d-imports-scene3d
    src_quick3d_imports_scene3d.depends = src_quick3d_render

    src_quick3d_imports_input.file = $$PWD/quick3d/imports/input/importsinput.pro
    src_quick3d_imports_input.target = sub-quick3d-imports-input
    src_quick3d_imports_input.depends = src_input src_quick3d_input

    src_quick3d_imports_logic.file = $$PWD/quick3d/imports/logic/importslogic.pro
    src_quick3d_imports_logic.target = sub-quick3d-imports-logic
    src_quick3d_imports_logic.depends = src_logic

    src_quick3d_imports_animation.file = $$PWD/quick3d/imports/animation/importsanimation.pro
    src_quick3d_imports_animation.target = sub-quick3d-imports-animation
    src_quick3d_imports_animation.depends = src_animation src_quick3d_animation

    src_quick3d_imports_extras.file = $$PWD/quick3d/imports/extras/importsextras.pro
    src_quick3d_imports_extras.target = sub-quick3d-imports-extras
    src_quick3d_imports_extras.depends = src_extras src_quick3d_extras

    src_quick3d_imports_scene2d.file = $$PWD/quick3d/imports/scene2d/importsscene2d.pro
    src_quick3d_imports_scene2d.target = sub-quick3d-imports-scene2d
    src_quick3d_imports_scene2d.depends = src_quick3d_scene2d
}

# Qt3D Scene Parser plugins
src_plugins_sceneparsers.file = $$PWD/plugins/sceneparsers/sceneparsers.pro
src_plugins_sceneparsers.target = sub-plugins-sceneparsers
src_plugins_sceneparsers.depends = src_render src_extras src_animation

# Qt3D Geometry Loader plugins
src_plugins_geometryloaders.file = $$PWD/plugins/geometryloaders/geometryloaders.pro
src_plugins_geometryloaders.target = sub-plugins-geometryloaders
src_plugins_geometryloaders.depends = src_render src_extras

qtHaveModule(quick) {
    # Qt3D Render plugins
    src_plugins_render.file = $$PWD/plugins/renderplugins/renderplugins.pro
    src_plugins_render.target = sub-plugins-render
    src_plugins_render.depends = src_render src_extras src_quick3d_render src_quick3d_scene2d
}

SUBDIRS += src_core

QT_FOR_CONFIG += 3dcore
include($$OUT_PWD/core/qt3dcore-config.pri)

qtConfig(qt3d-input): SUBDIRS += src_input
qtConfig(qt3d-logic): SUBDIRS += src_logic
qtConfig(qt3d-render): SUBDIRS += src_render
qtConfig(qt3d-animation) {
    SUBDIRS += src_animation
    qtConfig(qt3d-input): SUBDIRS += src_doc
}
qtConfig(qt3d-extras) {
    SUBDIRS += \
        src_extras \
        src_plugins_sceneparsers \
        src_plugins_geometryloaders
}

qtHaveModule(quick) {
    SUBDIRS += \
        src_quick3d_core \
        src_quick3d_core_imports

    qtConfig(qt3d-input) {
        SUBDIRS += \
            src_quick3d_input \
            src_quick3d_imports_input
    }
    qtConfig(qt3d-logic): SUBDIRS += src_quick3d_imports_logic
    qtConfig(qt3d-render) {
        SUBDIRS += \
            src_quick3d_render \
            src_quick3d_imports_render \
            src_quick3d_imports_scene3d

        qtConfig(qt3d-input) {
            src_quick3d_imports_scene3d.depends += src_input
            SUBDIRS += \
                src_quick3d_scene2d \
                src_quick3d_imports_scene2d
        }
        qtConfig(qt3d-logic): src_quick3d_imports_scene3d.depends += src_logic
        qtConfig(qt3d-animation): src_quick3d_imports_scene3d.depends += src_animation
    }
    qtConfig(qt3d-animation) {
        SUBDIRS += \
            src_quick3d_animation \
            src_quick3d_imports_animation
    }
    qtConfig(qt3d-extras) {
        SUBDIRS += \
            src_quick3d_extras \
            src_quick3d_imports_extras \
            src_plugins_render
    }
}
