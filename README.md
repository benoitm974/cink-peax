prereq:
	linaro arm-gcc 4.6 or 4.7
	(easier way is ubuntu >=12 with apt-get install build-essential gcc-arm-linux-gnueabi libncurses5-dev)

Build command:

kernel
======
	cd <kernel path>
	../abuild.sh clean
	../abuild.sh release

general notes:
==============
 most tweaks taken from excellent works from: https://github.com/varunchitre15/thunderzap_canvas_2/
 kernel source from wikogeek 4.0.4 for wiko cink peax

v0.5.1:
======
 - clkmanager etendu a MAINPLL et MEMPLL
 - boost de la MAINPLL au passage dans init.rc

v0.5:
=====
- clkmanager.c changé pour piloter la PLL
- cpufreq changé pour rajouté 1 fréquence: attention ici j'ai rapidement pris les fréquence de la version TM du proc mais ca ne veut rien dire je ne sais pas a combien tourne le proc! Tant que je n'ai pas décrypter les champs du registre, c'est de l'expérimental...
- init.c du boot.img modifier pour enregistrer la valeur d'overclock au boot. normalement tant que vous n'activer pas de setcpu ou autre la PLL et poussée mais le diviseur de fréquence est à F2 donc on boot en 1Ghz. et il faut un setCPU pour passer en O/C.
utiliser echo 0 0x000050a0 > /proc/clkmgr/pll_fsel en adb bash ou 50 et le coef le code d'origine est 43

download -> https://dl.dropbox.c...ot_peax_0_5.img

v0.4:
=====
governor: interactive(2xcore OK), conservative
 - SYNC control (attention patch controversé, desactivable:
    FSync Control which can be used to disable the fsync system calls for single files and filesystems (echo 0 > /sys/class/misc/fsynccontrol/fsync_enabled)) -> peut-être une optino à mettre dans une install de ROM ?
 - Tune CFS Parameters
 - sched: Disable GENTLE_FAIR_SLEEPERS
 - LZO compression
 - ioshed: SIO, VR and ZEN

v0.3:
=====
 WARNING: revert to stock sources as it seems I was too fast including patch
 - fix linaro 4.7 forcing inline function
 - fix flashlight, due to missing source from wiko
 - VFP compil with neon
 - Asynchronous I/O latency to a solid-state disk
 - governors: smartass2, zzmoove, pegasusq, lulzactiveq, hotplug
 - fix performance governor (fix AndreiLux )

V0.2:
====
 - Fsync control by Ezekeel
 - sched: enable ARCH_POWER
 - sched: Disable GENTLE_FAIR_SLEEPERS
 - Tune CFS Parameters

v0.1:
=====
 - CPU overclock O/C 1.2Ghz
 - performance: Turn CPUs online while active
 - VFP compil with neon
 - mm: Tweak vm dirty ratios
 - fs: Reduce vfs_cache_pressure to 20
 - Asynchronous I/O latency to a solid-state disk greatly increased.

