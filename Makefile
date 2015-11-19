CFLAGS = -std=c++14 -Wall -g -O3 -I`pwd`/src/

CORECC=clang++
CORELINK=clang++

ifeq ($(shell uname -m), armv7l)
#ubuntu mystery commands??
CFLAGS += -Wl,-Bsymbolic-functions -Wl,--no-as-needed
endif

OBJS := $(patsubst %.cc,%.o,$(wildcard src/*.cc))
OBJS += $(patsubst %.cc,%.o,$(wildcard src/syscall/*.cc))
OBJS += $(patsubst %.cc,%.o,$(wildcard src/abi/*.cc))

VDSO_OBJ= src/vdso/vdso_none.o

ifeq ($(shell uname -m), armv7l)
CFLAGS += -Wl,-Bsymbolic-functions -Wl,--no-as-needed -I/usr/include/arm-linux-gnueabi/
OBJS += src/cpu/ptimgarm.o
endif

ifeq ($(shell uname -m), x86_64)
OBJS += src/cpu/ptimgamd64.o src/cpu/ptimgi386.o		\
	src/cpu/ptamd64cpustate.o src/cpu/pti386cpustate.o
VDSO_OBJ= src/vdso/vdso_x64.o
endif
OBJS += $(VDSO_OBJ)

OBJS := $(patsubst src/%,obj/%,$(OBJS))


LIBTARGETS :=	bin/guestlib.a


.PHONY: all
all: $(LIBTARGETS)

.PHONY: clean
clean:
	rm -f $(LIBTARGETS) $(OBJS)

.PHONY: scan-build
scan-build:
	mkdir -p scan-out
	scan-build \
		`clang -cc1 -analyzer-checker-help | awk ' { print "-enable-checker="$1 } ' | grep '\.' | grep -v debug ` \
		-o `pwd`/scan-out make -j7 all

OUTDIRS=obj obj/cpu obj/syscall obj/vdso obj/abi
$(OBJS): | $(OUTDIRS)
$(OUTDIRS):
	mkdir -p $(OUTDIRS)

obj/%.o: src/%.s 
	gcc -c -o $@ $< $(CFLAGS)

obj/%.o: src/%.cc src/%.h 
	$(CORECC) -c -o $@ $< $(CFLAGS)

obj/%.o: src/%.cc 
	$(CORECC) -c -o $@ $< $(CFLAGS)

bin/guestlib.a: $(OBJS)
	ar r $@ $^