#!/bin/bash

# Note that in order to enable devtools 9 which contains a much newer gcc,
# environment variables such as PATH are updated with the "scl" command.
#
# For details, please see these URLs:
# https://access.redhat.com/documentation/en-us/red_hat_developer_toolset/9/html/user_guide/chap-gcc
# https://access.redhat.com/solutions/527703
#
# One way is to launch a new shell with the updated environment
# scl enable devtoolset-9 'bash'
#
# However it is much better to simply run the following "source" command:

source scl_source enable devtoolset-9


# Set MLIBQ workspace
export MLIBQ=/workspace/svc_quant_dev/MLIBQ

echo svc_quant_dev_git_build_and_test.sh started.

cd $MLIBQ

echo Starting update of workspace
git pull "origin" master
if [ $? -ne 0 ]
then
	echo "*** git pull failure! Check for local code modifications. ***"
	exit 1
fi

echo Starting build...
cd src

make -f Makefile.everything clean
make -f Makefile.everything -j8
if [ $? -ne 0 ]
then
	echo "*** Detected build failure! ***"
	exit 1
else
	echo "Build Successful. Starting TestNinja..."
	cd lib
	./TestNinja.py --threads 8
	if [ $? -ne 0 ]
	then
		echo "*** Detected test failures! ***"
		exit 1
	else
		echo "All builds and tests successful. Goodbye!"
	fi
fi


