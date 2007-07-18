SRCDIR := ../..
SUBDIRS :=

LIBNAME := widgets.a

SRCS := \
	widget.c \
	image.c \

include $(SRCDIR)/Makefile.common

CFLAGS += -I..

all:: depend $(LIBNAME)

clean::
	$(RM) $(LIBNAME)
