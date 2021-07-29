QT += quick-private network positioning-private positioningquick-private qml-private core-private gui-private

INCLUDEPATH += declarativemaps

PRIVATE_HEADERS += \
        declarativemaps/error_messages_p.h \
        declarativemaps/locationvaluetypehelper_p.h \
        declarativemaps/qdeclarativecirclemapitem_p.h \
        declarativemaps/qdeclarativegeocodemodel_p.h \
        declarativemaps/qdeclarativegeomaneuver_p.h \
        declarativemaps/qdeclarativegeomapcopyrightsnotice_p.h \
        declarativemaps/qdeclarativegeomapitembase_p.h \
        declarativemaps/qdeclarativegeomapitemgroup_p.h \
        declarativemaps/qdeclarativegeomapitemtransitionmanager_p.h \
        declarativemaps/qdeclarativegeomapitemview_p.h \
        declarativemaps/qdeclarativegeomapparameter_p.h \
        declarativemaps/qdeclarativegeomap_p.h \
        declarativemaps/qdeclarativegeomapquickitem_p.h \
        declarativemaps/qdeclarativegeomaptype_p.h \
        declarativemaps/qdeclarativegeoroutemodel_p.h \
        declarativemaps/qdeclarativegeoroute_p.h \
        declarativemaps/qdeclarativegeoroutesegment_p.h \
        declarativemaps/qdeclarativegeoserviceprovider_p.h \
        declarativemaps/qdeclarativepolygonmapitem_p.h \
        declarativemaps/qdeclarativepolylinemapitem_p.h \
        declarativemaps/qdeclarativerectanglemapitem_p.h \
        declarativemaps/qdeclarativeroutemapitem_p.h \
        declarativemaps/qgeomapitemgeometry_p.h \
        declarativemaps/qgeomapobject_p.h \
        declarativemaps/qgeomapobject_p_p.h \
        declarativemaps/qparameterizableobject_p.h \
        declarativemaps/qquickgeomapgesturearea_p.h

SOURCES += \
        declarativemaps/error_messages.cpp \
        declarativemaps/locationvaluetypehelper.cpp \
        declarativemaps/qdeclarativecirclemapitem.cpp \
        declarativemaps/qdeclarativegeocodemodel.cpp \
        declarativemaps/qdeclarativegeomaneuver.cpp \
        declarativemaps/qdeclarativegeomapcopyrightsnotice.cpp \
        declarativemaps/qdeclarativegeomap.cpp \
        declarativemaps/qdeclarativegeomapitembase.cpp \
        declarativemaps/qdeclarativegeomapitemgroup.cpp \
        declarativemaps/qdeclarativegeomapitemtransitionmanager.cpp \
        declarativemaps/qdeclarativegeomapitemview.cpp \
        declarativemaps/qdeclarativegeomapparameter.cpp \
        declarativemaps/qdeclarativegeomapquickitem.cpp \
        declarativemaps/qdeclarativegeomaptype.cpp \
        declarativemaps/qdeclarativegeoroute.cpp \
        declarativemaps/qdeclarativegeoroutemodel.cpp \
        declarativemaps/qdeclarativegeoroutesegment.cpp \
        declarativemaps/qdeclarativegeoserviceprovider.cpp \
        declarativemaps/qdeclarativepolygonmapitem.cpp \
        declarativemaps/qdeclarativepolylinemapitem.cpp \
        declarativemaps/qdeclarativerectanglemapitem.cpp \
        declarativemaps/qdeclarativeroutemapitem.cpp \
        declarativemaps/qgeomapitemgeometry.cpp \
        declarativemaps/qgeomapobject.cpp \
        declarativemaps/qparameterizableobject.cpp \
        declarativemaps/qquickgeomapgesturearea.cpp

load(qt_build_paths)
LIBS_PRIVATE += -L$$MODULE_BASE_OUTDIR/lib -lpoly2tri$$qtPlatformTargetSuffix() -lclip2tri$$qtPlatformTargetSuffix()
