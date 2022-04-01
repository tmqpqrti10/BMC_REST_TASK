# CMake version
cmake_minimum_required(
    VERSION 3.2
)

set(
    OUTPUT
    KETI-REST
)

set(
    SOURCE_FILES
    helper.cpp
    main.cpp
    req.cpp
    rhandler.cpp
)

set(CMAKE_CXX_FLAGS "-lssl -lcpprest -lcrypto -lmongoose -ljson-c -lm -lrt -lpthread -Wall -DBOOST_LOG_DYN_LIN -g -w -O3 -Wno-psabi -fpermissive -w -DMG_ENABLE_SSL -DMG_ENABLE_THREADS -DMG_ENABLE_HTTP_WEBSOCKET=0  -pthread -DMG_ENABLE_HTTP_STREAMING_MULTIPART -g -std=c++11") 

include_directories(
    ${INC_DIR}
)

add_executable(${OUTPUT} ${SOURCE_FILES})
target_link_libraries( ${OUTPUT}
    crypto 
    ssl 
    m
    rt
    pthread
    cpprest
    mongoose
    json-c
    ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/libapps.a
)