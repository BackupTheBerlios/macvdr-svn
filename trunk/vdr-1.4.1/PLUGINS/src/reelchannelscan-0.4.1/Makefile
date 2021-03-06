#
# Makefile for a Video Disk Recorder plugin
#
# $Id$

# The official name of this plugin.
# This name will be used in the '-P...' option of VDR to load the plugin.
# By default the main source file also carries this name.
#
#REELVDR=1

ifdef REELVDR
PLUGIN = channelscan
else
PLUGIN = reelchannelscan
endif

DEFINES+=-D_GNU_SOURCE 

### The version number of this plugin (taken from the main source file):

VERSION = $(shell grep 'static const char \*VERSION *=' channelscan.h | awk '{ print $$6 }' | sed -e 's/[";]//g')

### The C++ compiler and options:

CXX      ?= g++
#CXXFLAGS ?= -fPIC -O2 -Wall -Woverloaded-virtual -m32
CXXFLAGS ?= -fPIC -O2 -Wall -Woverloaded-virtual

### The directory environment:

ifdef REELVDR
DVBDIR = ../../../DVB
VDRDIR = ../../../vdr-current
else
VDRDIR = ../../..
endif

LIBDIR = ../../lib
TMPDIR = /tmp

### Allow user defined options to overwrite defaults:

-include $(VDRDIR)/Make.config

### The version number of VDR (taken from VDR's "config.h"):

APIVERSION = $(shell grep 'define APIVERSION ' $(VDRDIR)/config.h | awk '{ print $$3 }' | sed -e 's/"//g')

### The name of the distribution archive:

ARCHIVE = $(PLUGIN)-$(VERSION)
PACKAGE = vdr-$(ARCHIVE)

### Includes and Defines (add further entries here):

INCLUDES += -I$(VDRDIR)/include -I$(DVBDIR)/include

#DEFINES += -DPLUGIN_NAME_I18N='"$(PLUGIN)"' -DVDRDIR=\"$(VDRDIR)\" -DDEBUG_CHANNELSCAN
DEFINES += -DPLUGIN_NAME_I18N='"$(PLUGIN)"' -DVDRDIR=\"$(VDRDIR)\"

ifdef REELVDR
DEFINES += -DREELVDR
endif

### The object files (add further files here):

OBJS = channelscan.o csmenu.o filter.o i18n.o scan.o transponders.o 

### Implicit rules:

%.o: %.c
	$(CXX) $(CXXFLAGS) -c $(DEFINES) $(INCLUDES) $<

# Dependencies:

MAKEDEP = $(CXX) -MM -MG
DEPFILE = .dependencies
$(DEPFILE): Makefile
	@$(MAKEDEP) $(DEFINES) $(INCLUDES) $(OBJS:%.o=%.c) > $@

-include $(DEPFILE)

### Targets:

all: libvdr-$(PLUGIN).so

libvdr-$(PLUGIN).so: $(OBJS)
	$(CXX) $(CXXFLAGS) -dynamiclib -Wl,-single_module,-undefined,dynamic_lookup $(OBJS) $(LIBS) -o $@
	@cp $@ $(LIBDIR)/$@.$(APIVERSION)

dist: clean
	@-rm -rf $(TMPDIR)/$(ARCHIVE)
	@mkdir $(TMPDIR)/$(ARCHIVE)
	@cp -a * $(TMPDIR)/$(ARCHIVE)
	@tar czf $(PACKAGE).tgz -C $(TMPDIR) $(ARCHIVE)
	@-rm -rf $(TMPDIR)/$(ARCHIVE)
	@echo Distribution package created as $(PACKAGE).tgz

clean:
	@-rm -f $(OBJS) $(DEPFILE) *.so *.tgz core* *~
