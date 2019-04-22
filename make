#!/bin/bash
if [ "$1" == "" ]
then
	a=$(ls | grep build)
	if [ "$a" == "" ]
	then
		echo "Making directory"
		mkdir build
		cd build
		cmake ..
		cd ..
	fi
	cd build
	make
	cd ..
else
	if [ "$1" == "c" ]
	then
		rm -rf build
		mkdir build
		cd build
		cmake ..
		make
		cd ..
	fi
fi
