
TARGET = sg3_demo
SRCS = sg3_demo.c

include ../systype.mk
ifeq ($(SYSTYPE),mingw32)
SRCS += sg3_demo_mingw32.c
RSRCS = resources.rc
RDEPS = resources.h appicon.ico
else ifeq ($(SYSTYPE),linux)
SRCS += sg3_demo_linux.c
endif

include ../common.mk
