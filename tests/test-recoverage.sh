#!/bin/bash

BUILD_DIR=build-ut
HTML_DIR=html
REPORT_DIR=report

cd ../
rm -rf $BUILD_DIR
mkdir $BUILD_DIR
cd $BUILD_DIR

mkdir -p $HTML_DIR
mkdir -p $REPORT_DIR

cmake -DCMAKE_BUILD_TYPE=Debug ..
make -j 8

cd tests/

lcov -c -i -d ./ -o init.info
./dde_launcher_unit_test --gtest_output=xml:../$REPORT_DIR/ut-report_dde_launcher.xml
lcov -c -d ./ -o cover.info
lcov -a init.info -a cover.info -o total.info
lcov -r total.info "*/tests/*" "*/usr/include*" "*build/src*" '*dbusinterface*' '*tests*' '*dbusservices*' "*/dde-launcher_autogen/*" -o final.info


genhtml -o ../$HTML_DIR final.info

mv ../$HTML_DIR/index.html ../$HTML_DIR/cov_dde-launcher.html

mv asan.log* ../asan_dde-launcher.log
