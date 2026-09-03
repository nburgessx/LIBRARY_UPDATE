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

export ZIP_ARCHIVE=Linux_64.zip

echo svc_quant_dev_make_deploy_zip.sh started.

cd $MLIBQ
cd src/lib

echo "Clean up deployment folder..."
rm -rf deploy
rm -f $ZIP_ARCHIVE

echo "Copying GoogleTest and TestNinja..."
mkdir -p deploy/GoogleTest/resource/config
cp GOOGLE_TEST.exe *.py deploy/GoogleTest/.
cp -r ../../resource/config deploy/GoogleTest/resource/.

echo "Copying API libs..."
cp -r CSharp Java Python R deploy

echo "Copying a Python API example..."
cp ../../resource/api/PythonExamples/BondPriceAndYield/M_BONDPRICE.py deploy/Python/BondPriceExample.py

echo "Writing a short README.txt file..."
cat > deploy/Python/README.txt <<EOF
# Set your MLIBQ variable to point to this folder, which contains the python
MLIB_CLIENT_API and the "resource/config"  folder.

# e.g.
export MLIBQ=/workspace/svc_quant_dev/MLIBQ/src/lib/deploy/Python

# Make sure PATH and LD_LIBRARY_PATH is configured to run python3

# e.g.
export PATH=/opt/rh/rh-python35/root/usr/bin${PATH:+:${PATH}}
export LD_LIBRARY_PATH=/opt/rh/rh-python35/root/usr/lib64${LD_LIBRARY_PATH:+:${LD_LIBRARY_PATH}}

# Run the example python script
python3 BondPriceExample.py

EOF


echo "Making zip archive..."
cd deploy
zip -r ../$ZIP_ARCHIVE *

echo "Finished running Linux deploy / zip script. Goodbye!"
