TEMPLATE = subdirs

qtConfig(private_tests) {
    SUBDIRS += \
        flickableinterop \
        mousearea_interop \
        multipointtoucharea_interop \
        qquickdraghandler \
        qquickhoverhandler \
        qquickpinchhandler \
        qquickpointerhandler \
        qquickpointhandler \
        qquicktaphandler \
}
