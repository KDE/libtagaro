# - Try to find the Tagaro libraries
# Once done this will define
#  TAGARO_FOUND        - System has the Tagaro include directory and libraries.
#  TAGARO_INCLUDE_DIR  - The Tagaro include directory.
#  TAGARO_LIBRARIES    - Link this to use the Tagaro libraries

if(TAGARO_INCLUDE_DIR AND TAGARO_LIBRARIES)
	#in cache already
	set(Tagaro_FOUND TRUE)
else(TAGARO_INCLUDE_DIR AND TAGARO_LIBRARIES)
	#reset variables
	set(TAGARO_INCLUDE_DIRS)
	set(TAGARO_INCLUDE_DIR)
	set(TAGARO_LIBRARY)
	set(TAGARO_LIBRARIES)
	# locate includes
	find_path(TAGARO_INCLUDE_DIR NAMES tagaro/renderer.h HINTS
		${INCLUDE_INSTALL_DIR} ${KDE4_INCLUDE_DIR}
		${GNUWIN32_DIR}/include
	)
	set(TAGARO_INCLUDE_DIR ${TAGARO_INCLUDE_DIR})
	# locate libraries
	find_library(TAGARO_LIBRARIES NAMES tagarovisuals PATHS
		${LIB_INSTALL_DIR} ${KDE4_LIB_DIR}
		${GNUWIN32_DIR}/lib
	)
	set(TAGARO_LIBRARIES ${TAGARO_LIBRARIES})
	# handle find_package() arguments
	include(FindPackageHandleStandardArgs)
	find_package_handle_standard_args(Tagaro DEFAULT_MSG TAGARO_INCLUDE_DIR TAGARO_LIBRARIES)
	mark_as_advanced(TAGARO_INCLUDE_DIR TAGARO_LIBRARIES)
endif(TAGARO_INCLUDE_DIR AND TAGARO_LIBRARIES)
