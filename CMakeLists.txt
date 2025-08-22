cmake_minimum_required(VERSION 3.12)

project(Chat)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

if ( CMAKE_COMPILER_IS_GNUCC )
    set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wall -Wextra")
endif()
if ( MSVC )
    set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} /W4")
endif()

set (CHAT_LIBRARIES)

set (Qt6Modules Core Widgets Network Sql)
foreach (QtModule IN LISTS Qt6Modules)
    list (APPEND CHAT_LIBRARIES Qt6::${QtModule})
endforeach (QtModule)

find_package (Qt6 COMPONENTS ${Qt6Modules} REQUIRED LinguistTools)

file(GLOB_RECURSE SRC_FILES sources/*.cpp)
file(GLOB_RECURSE HEADERS_FILES sources/*.h)
file(GLOB_RECURSE RC_FILES sources/*.qrc)
file(GLOB_RECURSE UI_FILES sources/*.ui)

set (CMAKE_AUTOMOC ON)
set (CMAKE_AUTORCC ON)
set (CMAKE_AUTOUIC ON)

qt6_wrap_ui(QUI ${UI_FILES})
qt6_add_resources (QRC ${RC_FILES})

file(GLOB QM_FILES "${CMAKE_CURRENT_SOURCE_DIR}/translations/*.qm")

foreach(qm_file ${QM_FILES})
    get_filename_component(qm_filename ${qm_file} NAME)
    configure_file(
        ${qm_file}
        ${CMAKE_CURRENT_BINARY_DIR}/translations/${qm_filename}
        COPYONLY
    )
endforeach()

add_executable(Chat ${SRC_FILES} ${HEADERS_FILES} ${UI_FILES} ${RC_FILES})

target_include_directories(Chat
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/sources
)

target_link_libraries (Chat
PUBLIC
${CHAT_LIBRARIES}
)

file(GLOB STYLE_FILES "${CMAKE_SOURCE_DIR}/styles/*.qss")

foreach(style_file ${STYLE_FILES})
    get_filename_component(style_filename ${style_file} NAME)
    configure_file(${style_file} ${CMAKE_CURRENT_BINARY_DIR}/styles/${style_filename} COPYONLY)
endforeach()
