
TARGET = ctrl_demo
SRCS = ctrl_demo.c

include ../systype.mk
ifeq ($(SYSTYPE),mingw32)
SRCS += ctrl_demo_mingw32.c
RSRCS = resources.rc
RDEPS = resources.h appicon.ico
endif

include ../common.mk
