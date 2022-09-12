
#DEBUG=yes
ODBC=yes
VIM=yes

include ../Makefile.inc

# XXX MUST BE an absolute path as esxpost will be called by apache
ETCDIR=/home/esxadmin/etc

NAME=esxpost
PROG=$(NAME)$(EXE)
SRCS=main.c info.c post.c gendb_hosts.c gendb_farms.c utils.c clear.c conx.c
OBJS=$(SRCS:.c=.o)
CFLAGS+=-DETCDIR=\"$(ETCDIR)\"
GENDB=gendb -s esxadmin -u esxadmin_ro

all: $(PROG)

#$(OBJS): Makefile

info.c: gendb_hosts.h gendb_farms.h

gendb_farms.c gendb_farms.h:
	$(GENDB) -f s farms id

gendb_hosts.c gendb_hosts.h:
	$(GENDB) -f s hosts name

#mbs.c: gendb_mbs.h

#gendb_mbs.c gendb_mbs.h:
#	$(GENDB) -f s -s mbs -u mbs_ro -p "mbs2011readonly**" dbo.MBS hostname

$(PROG): $(OBJS) $(DEPS)
	$(CC) $(CFLAGS) -o $(PROG) $(OBJS) $(LIBS) 

$(OBJS): Makefile $(NAME).h

ETCFILES=datacenters dasoptions drsoptions licenses
install: $(PROG)
ifeq ($(OS),linux)
	sudo install -s -m 755 -o root -g root $(PROG) /usr/local/sbin
#	sudo mkdir -p $(ETCDIR)
#	sudo install -m 644 -o root -g root $(ETCFILES) $(ETCDIR)
	sudo install -m 755 -o root -g root cgi-esxpost /var/www/cgi-bin/esxpost
endif
#	sudo install -m 755 -o root -g root $(PROG) /var/www/html/tools/$(NAME)_$(ARCH)$(EXE)

clean:
	@rm -f $(PROG) $(OBJS) gendb*

cleanall: clean
	@rm -f *.log
