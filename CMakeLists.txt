project(libtagaro)

find_package(KDE4 4.4.80 REQUIRED)
include(KDE4Defaults)
include(MacroLibrary)

add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
include_directories(${QDBUS_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${KDE4_INCLUDES})

add_subdirectory(tagaro)
add_subdirectory(includes)

# allow libtagaro-based apps in this source tree to use the uninstalled includes correctly
include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/includes)
# this one is for <tagaro/settings.h>
include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_subdirectory(tools)
add_subdirectory(cmake)
add_subdirectory(doc)

install(FILES libtagaro_export.h DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel)
