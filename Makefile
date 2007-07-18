SRCDIR := .
SUBDIRS := src

include $(SRCDIR)/Makefile.common

distclean::
	$(RM) config.mak config.h config.log
