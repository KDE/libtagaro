# - Try to find the KGameVisuals library
# Once done this will define
#  KGAMEVISUALS_FOUND        - System has the KGameVisuals include directory.
#  KGAMEVISUALS_INCLUDE_DIRS - A list of the relevant KGame include directories. Allows the use of forwarding headers, e.g. <KgvRenderer>.
#  KGAMEVISUALS_INCLUDE_DIR  - The KGame include directory.
#  KGAMEVISUALS_LIBRARY      - Link this to use libkgamevisuals

if(KGAMEVISUALS_INCLUDE_DIR AND KGAMEVISUALS_LIBRARIES)
	#in cache already
	set(KGAMEVISUALS_FOUND TRUE)
else(KGAMEVISUALS_INCLUDE_DIR AND KGAMEVISUALS_LIBRARIES)
	#reset variables
	set(KGAMEVISUALS_INCLUDE_DIRS)
	set(KGAMEVISUALS_INCLUDE_DIR)
	set(KGAMEVISUALS_LIBRARY)
	set(KGAMEVISUALS_LIBRARIES)
	# locate includes
	find_path(KGAMEVISUALS_INCLUDE_DIR kgvrenderer.h
		${INCLUDE_INSTALL_DIR} ${KDE4_INCLUDE_DIR}
		${GNUWIN32_DIR}/include
	)
	set(KGAMEVISUALS_INCLUDE_DIR ${KGAMEVISUALS_INCLUDE_DIR})
	# locate libraries
	find_library(KGAMEVISUALS_LIBRARY NAMES kgamevisuals PATHS
		${LIB_INSTALL_DIR} ${KDE4_LIB_DIR}
		${GNUWIN32_DIR}/lib
	)
	set(KGAMEVISUALS_LIBRARY ${KGAMEVISUALS_LIBRARY})
	# handle find_package() arguments
	include(FindPackageHandleStandardArgs)
	find_package_handle_standard_args(KGAMEVISUALS DEFAULT_MSG KGAMEVISUALS_INCLUDE_DIR KGAMEVISUALS_LIBRARY)
	mark_as_advanced(KGAMEVISUALS_INCLUDE_DIR KGAMEVISUALS_LIBRARY)
endif(KGAMEVISUALS_INCLUDE_DIR AND KGAMEVISUALS_LIBRARIES)

#The ${KGAMEVISUALS_INCLUDE_DIR}/KDE addition is for the forwarding includes
if(NOT KGAMEVISUALS_INCLUDE_DIRS)
    set(KGAMEVISUALS_INCLUDE_DIRS ${KGAMEVISUALS_INCLUDE_DIR} ${KGAMEVISUALS_INCLUDE_DIR}/KDE)
    mark_as_advanced(KGAMEVISUALS_INCLUDE_DIRS)
endif(NOT KGAMEVISUALS_INCLUDE_DIRS)
