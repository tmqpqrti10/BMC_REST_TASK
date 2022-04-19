# CMake version
set(
    OUTPUT
    KETI-WDT
)

set(
    SOURCE_FILES
    watchdog.cpp
)


set(PROJECT_DIR ${ROOT_DIR}/source/AST2600_BMC)
set(INC_DIR ${PROJECT_DIR}/include)
set(LIB_DIR ${ROOT_DIR}/libs)
set(STATIC_LIBS_DIR ${PROJECT_DIR}/libs)

set(CMAKE_CXX_FLAGS "-lssl -lcpprest -lcrypto -lmongoose -ljson-c -lm -lrt -lpthread -Wall -DBOOST_LOG_DYN_LIN -g -w -O3 -Wno-psabi -fpermissive -w -DMG_ENABLE_SSL -DMG_ENABLE_THREADS -DMG_ENABLE_HTTP_WEBSOCKET=0  -pthread -DMG_ENABLE_HTTP_STREAMING_MULTIPART -g -std=c++11") 

include_directories(
    ${INC_DIR}
)

add_executable(${OUTPUT} ${SOURCE_FILES})
