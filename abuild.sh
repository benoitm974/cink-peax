#!/bin/bash

buildoutput=buildoutput
prebuilt=mediatek
makecore=-j27

export TARGET_PRODUCT=tinnoes77_s9091
export MTK_ROOT_CUSTOM=../mediatek/custom
export KBUILD_OUTPUT_SUPPORT=yes
export ARCH=arm
export CROSS_COMPILE=/usr/bin/arm-linux-gnueabi-



kerneldir=`pwd`

# Main starts here
while test -n "$1"; do
    case "$1" in
    release)
	make $makecore O=$buildoutput
    ;;
    clean)
	make clean
	mkdir -p $buildoutput/$prebuilt
	kerneldir=`pwd`
        cd ../$prebuilt
	find . -name *.module | tar -cf - -T - | tar -xf - -C $kerneldir/$buildoutput/$prebuilt
    ;;
    config)
	cp mediatek-configs $buildoutput/.config
    esac
    shift
done

