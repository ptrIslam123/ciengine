cmake_minimum_required(VERSION 3.18)

project(ciengine)

option(BUILD_SHARED_LIBS "Build using shared libraries" OFF)
option(BUILD_EXAMPLES "Build the examples" ON)
option(BUILD_TOOLS "Build the tools" ON)

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=thread -g") # enable thread sanitize

function(target_builder TARGET_NAME SRCS HDRS LIBS LIBS_DIR OUTTPUT_DIR)
    add_executable(${TARGET_NAME} ${SRCS} ${HDRS})

    target_link_libraries(${TARGET_NAME} PUBLIC ${LIBS})

    target_include_directories(${TARGET_NAME} PUBLIC ${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/include)

    target_link_directories(${TARGET_NAME} PUBLIC ${LIBS_DIR})

    set_target_properties(${TARGET_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${OUTPUT_DIR}")
endfunction(target_builder)

set(CMAKE_CXX_STANDARD 20)

#******************************************************* Building posnet library *******************************************************#
set(LIB_NAME ${PROJECT_NAME}_core)
set(LIB_DIRS ${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/include)
message(STATUS "CIENGINE_DIRS=${LIB_DIRS}")

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

set(LIB_HDRS 

)

set(LIB_SRCS 

)

message(STATUS "LIB_INSTALL_DIR=${CMAKE_BINARY_DIR}/lib")

if(BUILD_SHARED_LIBS)
    message(STATUS "BUILD_TYPE=DYNAMIC_LIBRARY")

    #add_library(${LIB_NAME} SHARED ${LIB_HDRS} ${LIB_SRCS})
else()
    message(STATUS "BUILD_TYPE=STATIC_LIBRARY")

    #add_library(${LIB_NAME} STATIC ${LIB_HDRS} ${LIB_SRCS})
endif()

#target_include_directories(${LIB_NAME} PUBLIC ${LIB_DIRS})

#set_target_properties(${LIB_NAME} PROPERTIES OUTPUT_NAME ${PROJECT_NAME})


#******************************************************* Build example dir *******************************************************#
if(BUILD_EXAMPLES)
    message(STATUS "BUILD_EXAMPLES=ON")

    target_builder("sync_checker" "examples/sync_checker.cpp" "" "" "" "examples")
endif()

#******************************************************* Build tools dir *******************************************************#
if(BUILD_TOOLS)
    message(STATUS "BUILD_TOOLS=ON")

    
endif()
