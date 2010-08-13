project(libkgame)

find_package(KDE4 4.4.80 REQUIRED)
include(KDE4Defaults)
include(MacroLibrary)

add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
include_directories(${QDBUS_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${KDE4_INCLUDES})
# make own library headers available for use by other libraries etc.
include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/includes
	${CMAKE_CURRENT_SOURCE_DIR}/visuals
	${CMAKE_CURRENT_SOURCE_DIR}/visuals/rendering
)

add_subdirectory(visuals)
add_subdirectory(includes)

install(FILES libkgame_export.h DESTINATION ${INCLUDE_INSTALL_DIR})
