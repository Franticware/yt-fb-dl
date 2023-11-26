#!/bin/bash
i686-w64-mingw32-windres yt-fb-dl.rc -O coff -o yt-fb-dl.res
CFLAGS="-O2 -fno-exceptions -fPIC"
i686-w64-mingw32-g++ -c main.cpp $CFLAGS -o main.o
i686-w64-mingw32-g++ -c zip.c $CFLAGS -o zip.o
i686-w64-mingw32-g++ -c util.cpp $CFLAGS -o util.o
LFLAGS="-lm -s -static-libstdc++ -static-libgcc -static -mwindows -lurlmon yt-fb-dl.res"
i686-w64-mingw32-g++ main.o zip.o util.o yt-fb-dl.res $LFLAGS -o yt-fb-dl.exe 2> >(grep -v "duplicate leaf")
