#!/bin/sh
# This script finds all headers (except for private headers) in the known
# library source trees, looks for exported classes inside there, and generates
# forward includes for all these classes.
# Calling syntax: sh autogenerate.sh ../visuals > CMakeLists.txt

echo 'install(FILES'
(
	while [ -d "$1" ]; do
		find "$1" -name \*.h -a \! -name \*_p.h | while read HEADERFILE; do
			# FIXME: This does not recognize namespaces (that doesn't hurt because we do not use any as of now)
			grep 'class KGAMEVISUALS_EXPORT' $HEADERFILE | sed 's/^.*EXPORT \([^ ]*\).*$/\1/' | while read CLASSNAME; do
				echo '#include <'$(basename $HEADERFILE)'>' > $CLASSNAME
				echo -en "\t"; echo "$CLASSNAME"
			done
		done
		shift
	done
	# add here hand-created headers
	echo -en "\t"; echo KgvSettings
) | sort
echo 'DESTINATION ${INCLUDE_INSTALL_DIR}/KDE COMPONENT Devel)'
