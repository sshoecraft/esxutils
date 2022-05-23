
BINDIR=/usr/local/bin

FILES=migrate smigrate to16

all:

install:
	for f in $(FILES) $(LIB_FILES); do sed -i -e 's:testing=1:testing=0:' $$f; done
	sudo install -m 755 -o root -g root $(FILES) $(BINDIR)

clean cleanall:
