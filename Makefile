default: all

.PHONY: all
.PHONY: clean
all clean:
	make -C log DEBUG=$(DEBUG) $@
	make -C mingw32 DEBUG=$(DEBUG) $@
	make -C tmcb DEBUG=$(DEBUG) $@
	make -C soil DEBUG=$(DEBUG) $@
	make -C gl3 DEBUG=$(DEBUG) $@
	make -C sg3_demo DEBUG=$(DEBUG) $@
#	make -C ctrl_demo DEBUG=$(DEBUG) $@
