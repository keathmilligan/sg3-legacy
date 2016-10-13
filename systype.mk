# detect system type
SPECS := $(shell gcc -v 2>&1)
ifeq ($(findstring mingw32,$(SPECS)), mingw32)
SYSTYPE = mingw32
else ifeq ($(findstring linux,$(SPECS)), linux)
SYSTYPE = linux
endif
$(info Target system: $(SYSTYPE))
