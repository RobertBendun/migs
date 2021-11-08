#!/bin/sh

if ! g++ -Wall -Wextra -std=c++20 -o migs migs.cc -O3; then
	exit 1
fi

rm -f output/*
./migs

cd output
for f in *ppm; do
	convert -quality 100 $f $(basename $f ppm)jpg
done
cd ..

ffmpeg -framerate 30 -i output/metaballs-%3d.jpg metaballs.gif
