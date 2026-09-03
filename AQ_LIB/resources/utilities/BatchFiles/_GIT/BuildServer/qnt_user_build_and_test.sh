#!/bin/bash

# Set paths to compiler etc
source /home/qnt_user/scripts/hirayama.bash_profile

# Set MLIB workspace
export MLIB=/home2/micfs/qnt_user/work/20181116/trunk_common

echo build_and_test.sh started.

cd $MLIB

head_revision=`svn info -r 'HEAD' | grep "Last Changed Rev" | awk '{ print $4 }'`
echo Starting update to revision $head_revision ...
svn update

echo Starting build...
cd src
make -f Makefile.everything clean
make -f Makefile.everything -j8
if [ $? -ne 0 ]
then
	echo *** Detected build failure! ***
	exit 1
else
	echo Build Successful. Starting TestNinja...
	cd lib
	./TestNinja.py --threads 8
	if [ $? -ne 0 ]
	then
		echo *** Detected test failures! ***
		exit 1
	else
		echo All builds and tests successful. Goodbye!
	fi
fi


