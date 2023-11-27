#!/bin/bash

./build.sh
rm -R yt-fb-dl
mkdir yt-fb-dl
cp yt-fb-dl.exe LICENSE README.md yt-fb-dl
rm yt-fb-dl.zip
zip -r yt-fb-dl.zip yt-fb-dl
