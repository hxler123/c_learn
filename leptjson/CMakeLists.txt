cmake_minimum_required(VERSION 3.12)
project(Letpjson)


file(GLOB_RECURSE LEPTJSON_SRC src/*.c)
message(${LEPTJSON_SRC})

# set(LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
# set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# set(LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/ss)
# set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/KK)

add_library(leptjson SHARED ${LEPTJSON_SRC})
set_target_properties(leptjson PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
# set(LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/ss)
set_target_properties(leptjson PROPERTIES PUBLIC_HEADER ${CMAKE_CURRENT_SOURCE_DIR}/src/leptjson.h)

install(TARGETS leptjson 
        LIBRARY DESTINATION /usr/local/lib
        PUBLIC_HEADER DESTINATION /usr/local/include
        )
