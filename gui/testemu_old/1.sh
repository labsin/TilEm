!#/bin/bash

echo "################ CLEAN ################"
make clean -f Makefile_testemu_old

echo ""
echo "################ MAKE ################"
make -f Makefile_testemu_old

echo ""
echo "################ TEST ################"
./testemu ti83_102.rom


