#!/bin/bash

# Set paths to compiler etc
source /home/qnt_user/scripts/hirayama.bash_profile

# Set MLIBQ workspace
export MLIBQ=/home2/micfs/qnt_user/work/20190409/

echo build_and_test.sh started.

cd $MLIBQ

echo Cleaning up workspace...
rm -rf src
rm -rf resource

# Install source from bamboo / git
# Until the necessary port is unblocked, we'll get the source this way
echo Unpacking source files from git...
cp /home/qnt_user/scripts/src.zip .       ; unzip src.zip
cp /home/qnt_user/scripts/resource.zip .  ; unzip resource.zip

echo Converting resource files to Linux line endings...
find resource -type f  -exec sed -i $'s/\r$//' {} \;

echo Starting build...
cd src
make -f Makefile.everything clean
make -f Makefile.everything -j16
if [ $? -ne 0 ]
then
	echo "*** Detected build failure! ***"
	exit 1
else
	echo "Build Successful. Starting TestNinja..."
	cd lib
	./TestNinja.py --threads 16
	if [ $? -ne 0 ]
	then
		echo "*** Detected test failures! ***"
		exit 1
	else
		echo "All builds and tests successful. Goodbye!"
	fi
fi


