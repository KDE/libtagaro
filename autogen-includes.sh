#!/bin/sh
# This script finds all headers (except for private headers) in the known
# library source trees, looks for exported classes inside there, and generates
# forward includes for all these classes.

[ -f autogen-includes.sh ] || ( echo "Call this script only from the root of the Tagaro source tree." >&2; exit 1 )

(
	echo '#NOTE: Use the autogen-includes.sh script to update this file.'
	echo 'install(FILES'
	(
		find tagaro/ -name \*.h -a \! -name \*_p.h | while read HEADERFILE; do
			grep 'class TAGARO_EXPORT' $HEADERFILE | sed 's/^.*EXPORT \([^ ]*\).*$/\1/' | while read CLASSNAME; do
				echo '#include <'$HEADERFILE'>' > includes/Tagaro/$CLASSNAME
				echo -en "\t"; echo "$CLASSNAME"
			done
		done
		# add here hand-created headers
		echo -en "\t"; echo Settings
	) | sort
	echo 'DESTINATION ${INCLUDE_INSTALL_DIR}/Tagaro COMPONENT Devel)'
) > includes/Tagaro/CMakeLists.txt
