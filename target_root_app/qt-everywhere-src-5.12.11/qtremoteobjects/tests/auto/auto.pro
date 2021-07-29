TEMPLATE = subdirs

sub_localsockettestserver.subdir = localsockettestserver
sub_localsockettestserver.target = sub-localsockettestserver

sub_integration.subdir = integration
sub_integration.target = sub-integration
sub_integration.depends = sub-localsockettestserver

SUBDIRS += \
    benchmarks \
    cmake \
    modelreplica \
    modelview \
    pods \
    proxy \
    repc \
    repcodegenerator \
    repparser \
    subclassreplica \
    sub_integration \
    sub_localsockettestserver

contains(QT_CONFIG, ssl): SUBDIRS += external_IODevice

qtHaveModule(qml): SUBDIRS += qml
qtConfig(process): SUBDIRS += integration_multiprocess proxy_multiprocess integration_external restart
