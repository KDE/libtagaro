project(libtagaro)

find_package(KDE4 4.4.80 REQUIRED)
include(KDE4Defaults)
include(MacroLibrary)
find_package(QCA2 REQUIRED)
find_package(OpenAL REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(SNDFILE REQUIRED sndfile)

add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS} ${QCA2_DEFINITIONS})
include_directories(${QDBUS_INCLUDE_DIRS} ${KDE4_INCLUDES} ${QCA2_INCLUDE_DIR} ${OPENAL_INCLUDE_DIR} ${SNDFILE_INCLUDE_DIRS})

# find uninstalled Tagaro includes correctly
include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}          # e.g. <tagaro/graphics/themeprovider.h>
	                                     # also <libtagaro_export.h>
	${CMAKE_CURRENT_SOURCE_DIR}/includes # e.g. <Tagaro/ThemeProvider>
	${CMAKE_CURRENT_BINARY_DIR}          # only <tagaro/settings.h>
)

add_subdirectory(tagaro)
add_subdirectory(includes)

add_subdirectory(tools)

macro_optional_add_subdirectory(examples)

install(FILES libtagaro_export.h DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel)

macro_display_feature_log()
