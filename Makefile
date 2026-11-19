TARGET = EBOOT
OBJS = main.o

INCDIR =
CFLAGS = -O2 -G0 -Wall
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

LIBS = -lpspdebug -lpspdisplay -lpspctrl -lpspkernel

BUILD_PRX = 1
PSP_FW_VERSION = 600

EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_TITLE = Musti Racing

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
