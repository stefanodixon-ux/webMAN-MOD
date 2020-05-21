#!/bin/bash

if [ -f polarssl-1.3.2/bin/libpolarssl-linux.a ]
then
	mv polarssl-1.3.2/bin/libpolarssl-linux.a polarssl-1.3.2/library/libpolarssl.a
fi

make -C polarssl-1.3.2/library

if [ -f Makefile.linux ]
then
	make -f Makefile.linux
#	mv Makefile Makefile.win
#	mv Makefile.linux Makefile
else
	make
fi
