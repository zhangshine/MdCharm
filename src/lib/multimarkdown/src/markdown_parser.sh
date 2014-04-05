#!/bin/sh
if [ -L $0 ]; then
	cd $(dirname $(readlink -f $0)) ;
else
	cd  $(dirname $0);
fi ;
../peg/leg/leg -o markdown_parser.c markdown_parser.leg
