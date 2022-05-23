
include ../Makefile.inc

BINDIR=/usr/local/bin
LIBDIR=/usr/local/lib/tools

SCRIPTS=vmconf
LIB_FILES=check_tools install_tools do_tools start_ntp do_ntp

all:

#	zcat key.gz > /tmp/vmconf.key; sudo install -m 644 -o root -g root /tmp/vmconf.key $(LIBDIR); rm -f /tmp/vmconf.key
install:
	sudo install -m 755 -o root -g root $(SCRIPTS) $(BINDIR)
	sudo mkdir -p $(LIBDIR)
	sudo install -m 755 -o root -g root $(LIB_FILES) $(LIBDIR)
	sudo su - root -c "for f in $(SCRIPTS); do sed -i -e 's:testing=1:testing=0:' $(BINDIR)/\$$f; done"
	sudo su - root -c "for f in $(LIB_FILES); do sed -i -e 's:testing=1:testing=0:' $(LIBDIR)/\$$f; done"

clean cleanall:
