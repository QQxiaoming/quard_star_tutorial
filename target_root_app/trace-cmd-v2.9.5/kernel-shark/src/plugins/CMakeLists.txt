message("\n src/plugins ...")

function(BUILD_PLUGIN)
    set(options )
    set(oneValueArgs NAME)
    set(multiValueArgs SOURCE)
    cmake_parse_arguments(ADD_PLUGIN "${options}"
                                     ${oneValueArgs}
                                     ${multiValueArgs}
                                     ${ARGN})

    message(STATUS ${ADD_PLUGIN_NAME})

    add_library(${ADD_PLUGIN_NAME} SHARED ${ADD_PLUGIN_SOURCE})
    set_target_properties(${ADD_PLUGIN_NAME} PROPERTIES PREFIX "plugin-")
    target_link_libraries(${ADD_PLUGIN_NAME} kshark)

endfunction()

set(PLUGIN_LIST "")
BUILD_PLUGIN(NAME sched_events
             SOURCE sched_events.c SchedEvents.cpp)
list(APPEND PLUGIN_LIST "sched_events default") # This plugin will be loaded by default
# list(APPEND PLUGIN_LIST "sched_events") # This plugin isn't loaded by default

BUILD_PLUGIN(NAME missed_events
             SOURCE missed_events.c MissedEvents.cpp)
list(APPEND PLUGIN_LIST "missed_events default") # This plugin will be loaded by default

install(TARGETS sched_events missed_events
        LIBRARY DESTINATION ${KS_PLUGIN_INSTALL_PREFIX})

set(PLUGINS ${PLUGIN_LIST} PARENT_SCOPE)
