
BINDIR=/var/www/cgi-bin

all:

SCRIPTS=buildupd genconf
#FILES=confmap patches *.xml
install:
	sudo install -o root -g root -m 755 $(SCRIPTS) $(BINDIR)
#	$(MAKE) $(MFLAGS) -C client install

clean cleanall:
