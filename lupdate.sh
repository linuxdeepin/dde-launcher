#!/bin/bash
cp ".transifexrc" ${HOME}/

cd ./translations

#rm -f dde-launcher.ts
#根据源码生成翻译英文翻译文件
lupdate ../src/ -ts -no-obsolete dde-launcher.ts

cd ../

lupdate ./ -ts -no-obsolete translations/dde-launcher.ts

tx push -s -b m23
