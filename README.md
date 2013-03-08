prereq:
	linaro arm-gcc 4.6 or 4.7
	(easier way is ubuntu >=12 with apt-get install build-essential gcc-arm-linux-gnueabi libncurses5-dev)

Build command:

kernel
======
	cd <kernel path>
	../abuild.sh clean
	../abuild.sh release
