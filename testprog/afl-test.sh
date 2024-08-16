#!/bin/bash

AFLGO="$HOME/aflgo"

rm -f /tmp/prog.txt
make BUILDDIR=obj-aflgo -f Makefile-without-cfg clean
mkdir -p obj-aflgo/temp

echo "Stage 1"
export SUBJECT=$PWD
export TMP_DIR="$PWD/obj-aflgo/temp"
export CC="$AFLGO/instrument/aflgo-clang"
export CXX="$AFLGO/instrument/aflgo-clang++"
export LDFLAGS=-lpthread
export ADDITIONAL="-targets=$TMP_DIR/BBtargets.txt -outdir=$TMP_DIR -flto -fuse-ld=gold -Wl,-plugin-opt=save-temps"

cat <<EOF > $TMP_DIR/BBtargets.txt
greeting.c:7
greeting.c:8
greeting.c:9
greeting.c:10
EOF

CFLAGS="$ADDITIONAL" CXXFLAGS="$ADDITIONAL" make BUILDDIR=obj-aflgo -f Makefile-without-cfg 

cp $TMP_DIR/BBnames.txt $TMP_DIR/BBnames.bk
cp $TMP_DIR/BBcalls.txt $TMP_DIR/BBcalls.bk

cat $TMP_DIR/BBnames.txt | rev | cut -d: -f2- | rev | sort | uniq > $TMP_DIR/BBnames2.txt && mv $TMP_DIR/BBnames2.txt $TMP_DIR/BBnames.txt
cat $TMP_DIR/BBcalls.txt | sort | uniq > $TMP_DIR/BBcalls2.txt && mv $TMP_DIR/BBcalls2.txt $TMP_DIR/BBcalls.txt

echo "Stage 2"
$AFLGO/distance/gen_distance_orig.sh $SUBJECT/obj-aflgo $TMP_DIR prog

rm -f obj-aflgo/prog
rm -f obj-aflgo/*.o

CFLAGS="-distance=$TMP_DIR/distance.cfg.txt" CXXFLAGS="-distance=$TMP_DIR/distance.cfg.txt" make BUILDDIR=obj-aflgo -f Makefile-without-cfg

cd obj-aflgo
mkdir in 
echo -n "crasH" > in/in0.txt

$AFLGO/afl-2.57b/afl-fuzz -m none -z exp -c 45m -i in -o out ./prog

