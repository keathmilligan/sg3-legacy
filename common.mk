# common makefile support
default: all

ifeq ($(SYSTYPE),)
include ../systype.mk
endif

# standard definitions
OBJDIR = obj
DEPSDIR = .dep
GCC = gcc
AR = ar
CPPFLAGS += -I../
ifneq ($(NOTMCB),1)
CPPFLAGS += -include ../tmcb/tmcb_inc.h
endif
ifeq ($(DEBUG),1)
	CFLAGS := $(CFLAGS) -Wall -g3 -O0 -fmessage-length=0 
else
	CFLAGS := $(CFLAGS) -Wall -g -O3 -fmessage-length=0
endif
DEPS = $(patsubst %.c,$(DEPSDIR)/%.d,$(SRCS))
OBJS = $(patsubst %.c,$(OBJDIR)/%.o,$(SRCS))
LDFLAGS := $(LDFLAGS) -L../gl3 -L../soil -L../log -L../tmcb
LIBS = -lgl3 -lsoil -ltmcb -llog
LIBDEPS = ../gl3/libgl3.a ../soil/libsoil.a ../log/liblog.a ../tmcb/libtmcb.a

# handle Windows/MingGW32-specific settings
ifeq ($(SYSTYPE),mingw32)
LDFLAGS := -mwindows $(LDFLAGS) -L../mingw32
LIBS := $(LIBS) -lsg3mingw32 -lopengl32 -lbfd -liberty -limagehlp -lglu32 -lgdi32 -lpthread -lintl
LIBDEPS += ../mingw/libmingw32.a
ifneq ($(suffix $(TARGET)),.a)
TARGET := $(TARGET).exe
endif
WINDRES = windres
ROBJ = $(patsubst %.rc,$(OBJDIR)/%.o,$(RSRCS))
OBJS += $(ROBJ)
$(ROBJ): $(RSRCS) $(RDEPS)
	$(WINDRES) resources.rc -o $@

# handle Linux-specific settings
else ifeq ($(SYSTYPE),linux)
LIBS := $(LIBS) -lX11 -lXi -lXmu -lglut -lGL -lGLU -lm -lpthread
else
$(error SYSTYPE not set)
endif

$(DEPSDIR):
	mkdir -p $@

$(OBJDIR):
	mkdir -p $@

$(OBJDIR)/%.o : %.c
	$(GCC) -c $(CPPFLAGS) $(CFLAGS) -MMD -MP -MT "$@" -MF $(DEPSDIR)/$*.d -o $@ $*.c

ifeq ($(suffix $(TARGET)),.a)
$(TARGET): $(OBJS)
	$(AR) rcs $@ $(OBJS)
else
$(TARGET): $(OBJS) $(LIBDEPS)
	$(GCC) -o $(TARGET) $(LDFLAGS) $(OBJS) $(LIBS) 
endif

.PHONY: all
all: $(DEPSDIR) $(OBJDIR) $(TARGET)

.PHONY: clean
clean:
	-rm -rf $(DEPSDIR) $(OBJDIR) $(TARGET)

-include $(DEPS)
