
BINDIR=/usr/local/bin
LIBDIR=/usr/local/lib/tools

FILES=farmcmd whordm vmkchk fixesx vmhist buildname genpass fromhist tohist showpo hostwwn farmwwn setguest
LIB_FILES=test_vmknics vmkchk_host doesxfix esxcmd esxscp

all:

#	for f in $(FILES);  do echo $$f; done
install:
	for f in $(FILES) $(LIB_FILES); do sed -i -e 's:testing=1:testing=0:' $$f; done
#	for f in $(FILES) $(LIB_FILES); do sed -i -e 's:verbose=1:verbose=0:' $$f; done
	sudo install -m 755 -o root -g root $(FILES) $(BINDIR)
	sudo mkdir -p $(LIBDIR)
	sudo install -m 755 -o root -g root $(LIB_FILES) $(LIBDIR)

clean cleanall:
