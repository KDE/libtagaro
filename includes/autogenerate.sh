#!/bin/sh
# This script finds all headers (except for private headers) in the known
# library source trees, looks for exported classes inside there, and generates
# forward includes for all these classes.
# NOTE: You have to run this from the directory where this script is located!

find ../visuals -name \*.h -a \! -name \*_p.h | while read HEADERFILE; do
	# FIXME: This does not recognize namespaces (that doesn't hurt because we do not use any as of now)
	grep 'class KGAMEVISUALS_EXPORT' $HEADERFILE | sed 's/^.*EXPORT \([^ ]*\).*$/\1/' | while read CLASSNAME; do
			echo '#include <'$(basename $HEADERFILE)'>' > $CLASSNAME
	done
done
