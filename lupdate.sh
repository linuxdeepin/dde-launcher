#!/bin/bash
cp ".transifexrc" ${HOME}/

cd ./translations

rm -f org.deepin.dde.launcher.ts
#根据源码生成翻译英文翻译文件
lupdate ../src/ -ts -no-obsolete org.deepin.dde.launcher.ts

cd ../

lupdate ./ -ts -no-obsolete translations/org.deepin.dde.launcher.ts

tx push -s -b m20
