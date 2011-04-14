project(libtagaro)

find_package(KDE4 4.4.80 REQUIRED)
include(KDE4Defaults)
include(MacroLibrary)
find_package(QCA2 REQUIRED)

macro_optional_find_package(OpenAL QUIET)
macro_log_feature(OPENAL_FOUND "OpenAL" "OpenAL (Open Audio Library) is a free software cross-platform audio API." "http://connect.creativelabs.com/openal" FALSE "" "Tagaro will use OpenAL for sound effects. If not found, Phonon is used.")

find_package(PkgConfig REQUIRED) # TODO: replace by FindSndFile.cmake from granatier
pkg_check_modules(SNDFILE sndfile)
#macro_optional_find_package(SndFile)
macro_log_feature(SNDFILE_FOUND "SndFile" "libsndfile is a C library written by Erik de Castro Lopo for reading and writing audio files." "http://www.mega-nerd.com/libsndfile/" FALSE "" "SndFile is needed for the OpenAL support of Tagaro.")

add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS} ${QCA2_DEFINITIONS})
include_directories(${KDE4_INCLUDES} ${QCA2_INCLUDE_DIR})

if (OPENAL_FOUND AND SNDFILE_FOUND)
	include_directories(${OPENAL_INCLUDE_DIR} ${SNDFILE_INCLUDE_DIRS})
	set(TAGAROAUDIO_LINKLIBS ${OPENAL_LIBRARY} ${SNDFILE_LIBRARIES})
	set(TAGAROAUDIO_BACKEND openal)
else (OPENAL_FOUND AND SNDFILE_FOUND)
	set(TAGAROAUDIO_LINKLIBS ${KDE4_PHONON_LIBS})
	set(TAGAROAUDIO_BACKEND phonon)
endif (OPENAL_FOUND AND SNDFILE_FOUND)
add_definitions(-DTAGAROAUDIO_BACKEND=${TAGAROAUDIO_BACKEND})

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
