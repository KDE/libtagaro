#!/bin/sh
# This script finds all headers (except for private headers) in the known
# library source trees, looks for exported classes inside there, and generates
# forward includes for all these classes.

# configuration
EXPORT_MACRO=TAGARO_EXPORT  # the macro which denotes exported classes
HEADER_DIR=tagaro           # the directory which contains the headers of your lib
NAMESPACE=Tagaro            # the namespace in which the classes reside
INCLUDE_DIR=includes/Tagaro # the directory which shall be filled with the pretty headers
INCLUDE_INSTALL_DIR='${INCLUDE_INSTALL_DIR}/Tagaro'
                            # the directory into which CMake shall install the pretty headers
MANUAL_HEADERS='Settings'   # specify manually created headers in this list (separated by spaces)

if [ ! -f $(basename $0) ]; then
	echo "Call this script only from the directory which contains it." >&2
	exit 1
fi

(
	echo "#NOTE: Use the $(basename $0) script to update this file."
	echo 'install(FILES'
	(
		find $HEADER_DIR/ -name \*.h -a \! -name \*_p.h | xargs grep "namespace ${NAMESPACE}" -l | while read HEADERFILE; do
			grep "class ${EXPORT_MACRO}" $HEADERFILE | sed "s/^.*${EXPORT_MACRO} \\([^ ]*\\).*$/\\1/" | while read CLASSNAME; do
				echo '#include <'$HEADERFILE'>' > $INCLUDE_DIR/$CLASSNAME
                                echo -e "\t${CLASSNAME}"
			done
		done
		for MANUAL_HEADER in $MANUAL_HEADERS; do
			if [ -n $MANUAL_HEADER ]; then
                                echo -e "\t${MANUAL_HEADER}"
			fi
		done
	) | sort
	echo "DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel)"
) > $INCLUDE_DIR/CMakeLists.txt
