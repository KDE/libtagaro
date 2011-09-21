project(libkgame)
cmake_minimum_required(VERSION 2.8)

set(KGAME_MAJOR_VERSION 0)
set(KGAME_MINOR_VERSION 1)
set(KGAME_PATCH_VERSION 0)
set(KGAME_SOVERSION 0)
set(KGAME_VERSION ${KGAME_MAJOR_VERSION}.${KGAME_MINOR_VERSION}.${KGAME_PATCH_VERSION})

find_package(KDE4 4.4.80 REQUIRED)
include(KDE4Defaults)
include(MacroLibrary)

macro_optional_find_package(OpenAL QUIET)
macro_log_feature(OPENAL_FOUND "OpenAL" "OpenAL (Open Audio Library) is a free software cross-platform audio API." "http://connect.creativelabs.com/openal" FALSE "" "KGame will use OpenAL for sound effects. If not found, Phonon is used.")

find_package(PkgConfig REQUIRED) # TODO: replace by FindSndFile.cmake from granatier
pkg_check_modules(SNDFILE sndfile)
#macro_optional_find_package(SndFile)
macro_log_feature(SNDFILE_FOUND "SndFile" "libsndfile is a C library written by Erik de Castro Lopo for reading and writing audio files." "http://www.mega-nerd.com/libsndfile/" FALSE "" "SndFile is needed for the OpenAL support of KGame.")

add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
include_directories(${KDE4_INCLUDES})

if (OPENAL_FOUND AND SNDFILE_FOUND)
	include_directories(${OPENAL_INCLUDE_DIR} ${SNDFILE_INCLUDE_DIRS})
	set(KGAMEAUDIO_LINKLIBS ${OPENAL_LIBRARY} ${SNDFILE_LIBRARIES})
	set(KGAMEAUDIO_BACKEND openal)
else (OPENAL_FOUND AND SNDFILE_FOUND)
	set(KGAMEAUDIO_LINKLIBS ${KDE4_PHONON_LIBS})
	set(KGAMEAUDIO_BACKEND phonon)
endif (OPENAL_FOUND AND SNDFILE_FOUND)
add_definitions(-DKGAMEAUDIO_BACKEND=${KGAMEAUDIO_BACKEND})

# find uninstalled KGame includes correctly
set(BUILD_TREE_INCLUDE_DIRS
	"${PROJECT_SOURCE_DIR}"          # e.g. <kgame/graphics/themeprovider.h>
	                                 # also <libkgame_export.h>
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
export(TARGETS kgame FILE ${PROJECT_BINARY_DIR}/KGameLibraryDepends.cmake)
# export the package for use from the build tree (this registers the build-tree
# with a global CMake registry)
export(PACKAGE KGame)

# create a KGameConfig.cmake file for use from the build tree
set(KGAME_INCLUDE_DIRS "${BUILD_TREE_INCLUDE_DIRS}")
set(KGAME_LIB_DIR "${PROJECT_BINARY_DIR}/kgame")
set(KGAME_CMAKE_DIR "${PROJECT_BINARY_DIR}")
configure_file(KGameConfig.cmake.in "${PROJECT_BINARY_DIR}/KGameConfig.cmake" @ONLY)
configure_file(KGameConfigVersion.cmake.in "${PROJECT_BINARY_DIR}/KGameConfigVersion.cmake" @ONLY)

# install the export set for use with the installed tree
install(EXPORT KGameLibraryDepends DESTINATION ${DATA_INSTALL_DIR}/kgame/cmake COMPONENT devel)

# install a KGameConfig.cmake file for use from the installed tree
set(KGAME_INCLUDE_DIRS ${INCLUDE_INSTALL_DIR})
set(KGAME_LIB_DIR ${LIB_INSTALL_DIR})
set(KGAME_CMAKE_DIR ${DATA_INSTALL_DIR}/kgame/cmake)
configure_file(KGameConfig.cmake.in "${PROJECT_BINARY_DIR}/InstallFiles/KGameConfig.cmake" @ONLY)
configure_file(KGameConfigVersion.cmake.in "${PROJECT_BINARY_DIR}/InstallFiles/KGameConfigVersion.cmake" @ONLY)
install(FILES
	"${PROJECT_BINARY_DIR}/InstallFiles/KGameConfig.cmake"
	"${PROJECT_BINARY_DIR}/InstallFiles/KGameConfigVersion.cmake"
DESTINATION ${KGAME_CMAKE_DIR})

install(FILES libkgame_export.h DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel)

macro_display_feature_log()
