#!/bin/sh
mkdir -p package
cd package
rm -f *
cp ../README.md ./
cp ../LICENSE ./

for x in Q3A RTCWMP RTCWSP WET JAMP JASP JK2MP JK2SP STVOYHM STVOYSP SOF2MP STEF2 MOHAA MOHBT MOHSH QUAKE2; do
  cp ../bin/release-$x/x86/qadmin_qmm_$x.so ./
  cp ../bin/release-$x/x86_64/qadmin_qmm_x86_64_$x.so ./
done 

cd ..
