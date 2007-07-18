SRCDIR := ../..
SUBDIRS :=

LIBNAME := widgets.a

SRCS := \
	widget.c \

include $(SRCDIR)/Makefile.common

CFLAGS += -I..

all:: depend $(LIBNAME)

clean::
	$(RM) $(LIBNAME)
