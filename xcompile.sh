#!/bin/bash
# Cross-compiling script - generates Win 32bit static binaries
# See https://stackoverflow.com/questions/10934683/how-do-i-configure-qt-for-cross-compilation-from-linux-to-windows-target/13211922#13211922

MXE_ROOT="/home/simon/Projekte/qt/mxe"
$MXE_ROOT/usr/bin/i686-w64-mingw32.static-qmake-qt5

export PATH=$MXE_ROOT/usr/bin:$PATH

make -j 8

