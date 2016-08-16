#!/bin/bash
# Cross-compiling script - generates Win 32bit static binaries
# See https://stackoverflow.com/questions/10934683/how-do-i-configure-qt-for-cross-compilation-from-linux-to-windows-target/13211922#13211922

MXE_ROOT="/home/simon/Projekte/qt/mxe"

export PATH=$MXE_ROOT/usr/bin:$PATH

case "$1" in
    release)
        echo "Creating RELEASE build"
        make clean

        # Create 64 bit build and package it
        $MXE_ROOT/usr/bin/x86_64-w64-mingw32.static-qmake-qt5
        make -j 8

        cd release
        rm NormalmapGenerator_win64.zip
        zip NormalmapGenerator_win64 NormalmapGenerator.exe
        cd ..

        # Cleanup
        make clean

        # Create 32 bit build and package it
        $MXE_ROOT/usr/bin/i686-w64-mingw32.static-qmake-qt5
        make -j 8

        cd release
        rm NormalmapGenerator_win32.zip
        zip NormalmapGenerator_win32 NormalmapGenerator.exe
        cd ..
        
        make clean
        echo "RELEASE build done."
        ;;
    *)
        echo "Creating DEBUG build"
        $MXE_ROOT/usr/bin/x86_64-w64-mingw32.static-qmake-qt5
        make -j 8
        echo "DEBUG build done."
esac


