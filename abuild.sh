#!/bin/bash

buildoutput=buildoutput
prebuilt=mediatek
makecore=-j27a
mkbootpath=../mkbootimg

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
    ;;
    unpack)
	cd $mkbootpath
	./unpack-MT65xx.pl boot.img
    ;;
    pack)
	cd $mkbootpath
	dd if=boot.img-kernel.img of=boot_head_512.img count=1
	dd if=../kernel/$buildoutput/arch/arm/boot/zImage of=boot_head_512.img seek=1	
	./repack-MT65xx.pl -boot boot_head_512.img boot.img-ramdisk monboot.img
    esac
    shift
done

