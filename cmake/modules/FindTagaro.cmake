# - Try to find the Tagaro library
# Once done this will define
#  TAGARO_FOUND        - System has the Tagaro include directory and library.
#  TAGARO_INCLUDE_DIR  - The Tagaro include directory.
#  TAGARO_LIBRARY      - Link this to use the Tagaro library.

if(TAGARO_INCLUDE_DIR AND TAGARO_LIBRARY)
	#in cache already
	set(Tagaro_FOUND TRUE)
else(TAGARO_INCLUDE_DIR AND TAGARO_LIBRARY)
	#reset variables
	set(TAGARO_INCLUDE_DIRS)
	set(TAGARO_INCLUDE_DIR)
	set(TAGARO_LIBRARY)
	# locate includes
	find_path(TAGARO_INCLUDE_DIR NAMES tagaro/settings.h HINTS
		${INCLUDE_INSTALL_DIR} ${KDE4_INCLUDE_DIR}
		${GNUWIN32_DIR}/include
	)
	set(TAGARO_INCLUDE_DIR ${TAGARO_INCLUDE_DIR})
	# locate libraries
	find_library(TAGARO_LIBRARY NAMES tagaro PATHS
		${LIB_INSTALL_DIR} ${KDE4_LIB_DIR}
		${GNUWIN32_DIR}/lib
	)
	set(TAGARO_LIBRARY ${TAGARO_LIBRARY})
	# handle find_package() arguments
	include(FindPackageHandleStandardArgs)
	find_package_handle_standard_args(Tagaro DEFAULT_MSG TAGARO_INCLUDE_DIR TAGARO_LIBRARY)
	mark_as_advanced(TAGARO_INCLUDE_DIR TAGARO_LIBRARY)
endif(TAGARO_INCLUDE_DIR AND TAGARO_LIBRARY)
