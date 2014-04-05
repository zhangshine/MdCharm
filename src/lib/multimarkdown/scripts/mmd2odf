#!/bin/sh
#
# mmd2odf --- MultiMarkdown convenience script
#	<http://fletcherpenney.net/multimarkdown/>
#	Fletcher T. Penney
#
# Pass arguments on to the binary to convert text to OpenDocument
#

# Be sure to include multimarkdown in our PATH
export PATH="/usr/local/bin:$PATH"

if [ $# = 0 ]
then
	multimarkdown -t odf
else
until [ "$*" = "" ]
do
	multimarkdown -b -t odf "$1"
	shift
done
fi
