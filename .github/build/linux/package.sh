#!/bin/sh
mkdir -p package
cd package
rm -f *
cp ../README.md ./
cp ../LICENSE ./

for f in COD11MP CODMP CODUOMP JAMP JASP JK2MP JK2SP MOHAA MOHBT MOHSH Q3A QUAKE2 RTCWMP RTCWSP SIN SOF2MP STEF2 STVOYHM STVOYSP WET; do
  cp ../bin/release-$f/x86/qadmin_qmm_$f.so ./
  cp ../bin/release-$f/x86_64/qadmin_qmm_x86_64_$f.so ./
done

cd ..
