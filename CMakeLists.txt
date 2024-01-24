cmake_minimum_required(VERSION "3.20")
project("SeqMakeDebugger" VERSION 1.0.0 LANGUAGES CXX)

FILE(GLOB SRCS "./src/*.cpp")
FILE(GLOB INCLUDES "./include/*.hpp")
add_executable("SeqMakeDebugger" ${SRCS} ${INCLUDES})

target_compile_features("SeqMakeDebugger" PRIVATE cxx_std_20)

target_include_directories("SeqMakeDebugger" PRIVATE "./include/")

target_compile_options("SeqMakeDebugger" PRIVATE
  "-Wall"
  "-Wextra"
  "-Werror"
  "-finput-charset=UTF-8"
  "-fexec-charset=CP932"
  "-municode"
)


# Link SeqMaker

set(SeqMaker_DIR "./libs/SeqMaker/cmake")
find_package(SeqMaker REQUIRED)

target_link_libraries("SeqMakeDebugger" PRIVATE SeqMaker::SeqMaker)

add_custom_command(TARGET "SeqMakeDebugger" POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
    "${PROJECT_SOURCE_DIR}/libs/SeqMaker/lib/libSeqMaker.dll"
    $<TARGET_FILE_DIR:SeqMakeDebugger>
)
