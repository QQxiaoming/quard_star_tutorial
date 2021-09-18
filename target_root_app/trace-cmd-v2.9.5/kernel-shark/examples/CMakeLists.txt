message("\n examples ...")

message(STATUS "dataload")
add_executable(dload          dataload.c)
target_link_libraries(dload   kshark)

message(STATUS "datafilter")
add_executable(dfilter          datafilter.c)
target_link_libraries(dfilter   kshark)

message(STATUS "datahisto")
add_executable(dhisto          datahisto.c)
target_link_libraries(dhisto   kshark)

message(STATUS "confogio")
add_executable(confio          configio.c)
target_link_libraries(confio   kshark)

message(STATUS "dataplot")
add_executable(dplot          dataplot.cpp)
target_link_libraries(dplot   kshark-plot)

if (Qt5Widgets_FOUND)

    message(STATUS "widgetdemo")
    add_executable(widgetdemo          widgetdemo.cpp)
    target_link_libraries(widgetdemo   kshark-gui)

    message(STATUS "cmd_split")
    add_executable(cmd_split           cmd_split.cpp)
    target_link_libraries(cmd_split    kshark-gui)

endif (Qt5Widgets_FOUND)
