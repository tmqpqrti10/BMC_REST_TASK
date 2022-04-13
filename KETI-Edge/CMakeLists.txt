# Variable definition
## Execute file
set(
    OUTPUT
    KETI-Edge
)

## Source file
set(
    SOURCE_FILES 
    msg_session.cpp
    rmcp.cpp
    khandler.cpp
    keti-edge.cpp
    handler.cpp
    hwcontrol.cpp
    # resource.cpp
    # util.cpp
    # record.cpp
    rest_handler.cpp
    init.cpp
    lssdp.cpp
    psu_server.cpp
)

# Directory setting
set(Boost_INCLUDE_DIR ${ROOT_DIR}/BMC_SDK/output/build/boost-1.73.0)
set(Boost_LIBRARY_DIR ${ROOT_DIR}/BMC_SDK/output/build/boost-1.73.0/stage/lib)

# Linkinking
## Header -> lib
include_directories(
    ${Boost_INCLUDE_DIR}
    ${INC_DIR}
)
## Source
link_directories(
    ${Boost_LIBRARY_DIR} 
)

# Build
add_executable(${OUTPUT} ${SOURCE_FILES})
target_link_libraries( ${OUTPUT}
    crypto 
    ssl 
    m
    rt
    uuid
    pthread
    pam
    pam_misc
    boost_system
    boost_log
    boost_log_setup
    boost_thread
    cpprest
    sqlite3
    stdc++fs
    ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/libapps.a
)

