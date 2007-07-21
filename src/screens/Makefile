SRCDIR := ../..
SUBDIRS :=

LIBNAME := screens.a

SRCS := \
	screen.c \
	screen_main.c \

include $(SRCDIR)/Makefile.common

CFLAGS += -I..

all:: depend $(LIBNAME)

clean::
	$(RM) $(LIBNAME)
