#######################################
# Sepcify source files
#######################################
include $(MTK_ROOT_BUILD)/Makefile

lib_obj := src/sec_mod.o auth/sec_wrapper.o
ifeq ($(MTK_EMMC_SUPPORT),yes)
    ifeq ($(strip $(KBUILD_OUTPUT_SUPPORT)),yes)
        lib_obj += $(call to-root,$(obj))$(obj)/prebuilt/libasp_EMMC.a
    else
        lib_obj += prebuilt/libasp_EMMC.a
    endif
else 
    ifeq ($(strip $(KBUILD_OUTPUT_SUPPORT)),yes)
        lib_obj += $(call to-root,$(obj))$(obj)/prebuilt/libasp_NAND.a
    else
        lib_obj += prebuilt/libasp_NAND.a	
    endif
endif

ifeq ($(strip $(KBUILD_OUTPUT_SUPPORT)),yes)
  lib_obj += $(call to-root,$(obj))$(obj)/prebuilt/libasp_auth.a
else  
  lib_obj += prebuilt/libasp_auth.a
endif
obj-m += sec.o

# HEADER FILES
EXTRA_CFLAGS += -I$(src) -I$(src)/inc

# INCLUDE ASP LIBRARY
sec-y += $(lib_obj)
