
include ../Makefile.inc

all:

FILES=vminfo hostinfo farminfo whichvc cvm fcvm aaminfo wherevm farmuse
install:
	sudo install -m 755 -o root -g root $(FILES) /usr/local/bin/
	sudo sed -i -e 's:^my $$debug = 1:my $$debug = 0:' /usr/local/bin/vminfo
	sudo sed -i -e 's:^my $$debug = 1:my $$debug = 0:' /usr/local/bin/hostinfo
	sudo install -m 755 -o root -g root dsinfo $$HOME/bin/

clean cleanall:
