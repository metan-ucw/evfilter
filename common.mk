include $(TOPDIR)/config.mk

DEPFILES=$(subst .c,.dep,$(CSOURCES))
OBJECTS=$(subst .c,.o,$(CSOURCES))
CLEAN+=$(DEPFILES) $(OBJECTS) $(BINARIES)

all: $(OBJECTS) $(SUBDIRS) $(TARGETS) $(BINARIES)

$(OBJECTS): %.o: %.c
ifdef VERBOSE
	$(CC) $(CFLAGS) -c $< -o $@
else
	@echo "CC  $@"
	@$(CC) $(CFLAGS) -c $< -o $@
endif

$(BINARIES):
ifdef VERBOSE
	$(CC) $(CFLAGS) $(LDFLAGS) $^ $(LDLIBS) -o $@
else
	@echo "LD  $@"
	@$(CC) $(CFLAGS) $(LDFLAGS) $^ $(LDLIBS) -o $@
endif

# Automatic source code dependencies
$(OBJECTS): $(DEPFILES)

$(DEPFILES): %.dep: %.c
ifdef VERBOSE
	$(CC) -MM $(CFLAGS) $< -o $@
else
	@echo "DEP $@"
	@$(CC) -MM $(CFLAGS) $< -o $@
endif

-include $(DEPFILES)

# Subdirectory handling
.PHONY: $(SUBDIRS) $(TARGETS)

$(SUBDIRS):
ifdef VERBOSE
	$(MAKE) -C $@ $(MAKECMDGOALS)
else
	@echo "DIR $@"
	@$(MAKE) --no-print-directory -C $@ $(MAKECMDGOALS)
endif

clean: $(SUBDIRS)
ifdef VERBOSE
	rm -f $(CLEAN)
else
	@echo "RM  $(CLEAN)"
	@rm -f $(CLEAN)
endif

# Installation
install: $(SUBDIRS)

ifdef DESTDIR
BINDIR=$(DESTDIR)/$(bindir)
INCLUDEDIR=$(DESTDIR)/$(includedir)/evfilter/
LIBDIR=$(DESTDIR)/$(libdir)
else
BINDIR=$(bindir)
INCLUDEDIR=$(includedir)/evfilter/
LIBDIR=$(libdir)
endif

install:
ifdef INSTALL_BINARIES
	install -d $(BINDIR)
	for i in $(INSTALL_BINARIES); do install -m 775 $$i $(BINDIR); done
endif
ifdef INSTALL_HEADERS
	install -d $(INCLUDEDIR)
	for i in $(INSTALL_HEADERS); do install -m 644 $$i $(INCLUDEDIR); done
endif
ifdef INSTALL_LIBRARIES
	install -d $(LIBDIR)
	for i in $(INSTALL_LIBRARIES); do cp -d $$i $(LIBDIR); done
endif
