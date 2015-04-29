#!/bin/sh
CC=${CC:-gcc}
dir=$1
shift
if [ "$dir" != "" ]
then
	dir="$dir/"
fi
$CC -MM -MG "$@" | sed -e "s@\(.*\).o:@$dir\1.o $dir\1.d:@"
