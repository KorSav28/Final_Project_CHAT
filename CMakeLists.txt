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

if(NOT Qt6_LUPDATE_EXECUTABLE)
    set(Qt6_LUPDATE_EXECUTABLE "C:/NEW_QT/6.9.1/msvc2022_64/bin/lupdate.exe")
endif()

#здесь происходит создание файлов переводов и установка зависимостей,
#чтобы эти файлы генерировались во время билда проекта
set(TS_FILES
 ${CMAKE_CURRENT_SOURCE_DIR}/translations/my_ru.ts)
file(GLOB_RECURSE TS_SOURCES "*.cpp" "*.h" ".ui")
add_custom_target(translations)
#по сути, при билде для каждого .ts файла вызывается
#утилита lupdate.exe
foreach(_ts_file ${TS_FILES})
 get_filename_component(_ts_name ${_ts_file} NAME_WE)
 add_custom_command(
  OUTPUT "${_ts_file}"
   DEPENDS ${TS_SOURCES}
#вот тут как раз описывается путь к вызываемой утилите lupdate.exe
#и ее входные аргументы:
   COMMAND ${Qt6_LUPDATE_EXECUTABLE} -recursive
   ${CMAKE_CURRENT_SOURCE_DIR} -ts ${_ts_file}
  )
 add_custom_target(translate_${_ts_name} DEPENDS ${_ts_file})
 add_dependencies(translations translate_${_ts_name})
endforeach()

#подключаем файлы трансляции(переводов)
qt6_add_translation(QM_FILES ${TS_FILES})

add_executable(Chat ${QM_FILES} ${SRC_FILES} ${HEADERS_FILES} ${UI_FILES} ${RC_FILES})

target_link_libraries (Chat
PUBLIC
${CHAT_LIBRARIES}
)
