project(libtagaro)
cmake_minimum_required(VERSION 2.8)

set(TAGARO_MAJOR_VERSION 0)
set(TAGARO_MINOR_VERSION 1)
set(TAGARO_PATCH_VERSION 0)
set(TAGARO_SOVERSION 0)
set(TAGARO_VERSION ${TAGARO_MAJOR_VERSION}.${TAGARO_MINOR_VERSION}.${TAGARO_PATCH_VERSION})

find_package(KDE4 4.4.80 REQUIRED)
include(KDE4Defaults)
include(MacroLibrary)

macro_optional_find_package(OpenAL QUIET)
macro_log_feature(OPENAL_FOUND "OpenAL" "OpenAL (Open Audio Library) is a free software cross-platform audio API." "http://connect.creativelabs.com/openal" FALSE "" "Tagaro will use OpenAL for sound effects. If not found, Phonon is used.")

find_package(PkgConfig REQUIRED) # TODO: replace by FindSndFile.cmake from granatier
pkg_check_modules(SNDFILE sndfile)
#macro_optional_find_package(SndFile)
macro_log_feature(SNDFILE_FOUND "SndFile" "libsndfile is a C library written by Erik de Castro Lopo for reading and writing audio files." "http://www.mega-nerd.com/libsndfile/" FALSE "" "SndFile is needed for the OpenAL support of Tagaro.")

add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
include_directories(${KDE4_INCLUDES})

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
set(BUILD_TREE_INCLUDE_DIRS
	"${PROJECT_SOURCE_DIR}"          # e.g. <kgame/graphics/themeprovider.h>
	                                 # also <libtagaro_export.h>
	"${PROJECT_SOURCE_DIR}/includes" # e.g. <KGame/ThemeProvider>
	"${PROJECT_BINARY_DIR}"          # only <kgame/settings.h>
)
include_directories(${BUILD_TREE_INCLUDE_DIRS})

add_subdirectory(kgame)
add_subdirectory(includes)
add_subdirectory(tools)
macro_optional_add_subdirectory(examples)

# the following adapted from http://www.vtk.org/Wiki/CMake/Tutorials/How_to_create_a_ProjectConfig.cmake_file

# add all targets to the build-tree export set
export(TARGETS tagaro FILE ${PROJECT_BINARY_DIR}/TagaroLibraryDepends.cmake)
# export the package for use from the build tree (this registers the build-tree
# with a global CMake registry)
export(PACKAGE Tagaro)

# create a TagaroConfig.cmake file for use from the build tree
set(TAGARO_INCLUDE_DIRS "${BUILD_TREE_INCLUDE_DIRS}")
set(TAGARO_LIB_DIR "${PROJECT_BINARY_DIR}/tagaro")
set(TAGARO_CMAKE_DIR "${PROJECT_BINARY_DIR}")
configure_file(TagaroConfig.cmake.in "${PROJECT_BINARY_DIR}/TagaroConfig.cmake" @ONLY)
configure_file(TagaroConfigVersion.cmake.in "${PROJECT_BINARY_DIR}/TagaroConfigVersion.cmake" @ONLY)

# install the export set for use with the installed tree
install(EXPORT TagaroLibraryDepends DESTINATION ${DATA_INSTALL_DIR}/tagaro/cmake COMPONENT devel)

# install a TagaroConfig.cmake file for use from the installed tree
set(TAGARO_INCLUDE_DIRS ${INCLUDE_INSTALL_DIR})
set(TAGARO_LIB_DIR ${LIB_INSTALL_DIR})
set(TAGARO_CMAKE_DIR ${DATA_INSTALL_DIR}/tagaro/cmake)
configure_file(TagaroConfig.cmake.in "${PROJECT_BINARY_DIR}/InstallFiles/TagaroConfig.cmake" @ONLY)
configure_file(TagaroConfigVersion.cmake.in "${PROJECT_BINARY_DIR}/InstallFiles/TagaroConfigVersion.cmake" @ONLY)
install(FILES
	"${PROJECT_BINARY_DIR}/InstallFiles/TagaroConfig.cmake"
	"${PROJECT_BINARY_DIR}/InstallFiles/TagaroConfigVersion.cmake"
DESTINATION ${TAGARO_CMAKE_DIR})

install(FILES libtagaro_export.h DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel)

macro_display_feature_log()
