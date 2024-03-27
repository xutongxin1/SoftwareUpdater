cmake_minimum_required(VERSION 3.25)
project(SoftwareUpdater)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)


find_package(Qt6 COMPONENTS
        Core Widgets Network
        REQUIRED)


file(GLOB HEADER "*.h")
file(GLOB RSOURCE "*.cpp")
file(GLOB UI "*.ui")
file(GLOB QRC "*.qrc")
#file(GLOB_RECURSE Resource "./thirdLib/qmetarial/resources.qrc")
file(GLOB_RECURSE SOURCES ${SOURCES} "./config/*.ini")

if (${CMAKE_BUILD_TYPE} STREQUAL "Release")
    add_executable(${PROJECT_NAME} WIN32 ${HEADER} ${RSOURCE} ${UI} ${QRC} ${Resource})
else ()
    add_executable(${PROJECT_NAME} ${HEADER} ${RSOURCE} ${UI} ${QRC} ${Resource})
endif ()
add_subdirectory(api)

target_link_libraries(SoftwareUpdater
        Qt::Core Qt::Widgets Qt::Network
        )

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND cp.exe -r -f ${PROJECT_SOURCE_DIR}/config ${CMAKE_CURRENT_BINARY_DIR}
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/thirdPartyTool/cp
        )

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND cp.exe -r -f ${PROJECT_SOURCE_DIR}/thirdPartyTool/cp ${CMAKE_CURRENT_BINARY_DIR}
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/thirdPartyTool/cp
        )