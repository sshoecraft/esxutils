
DEBUG=yes
VIM=yes
UTIL=yes

ifeq ($(strip $(BUILDROOT)),)
SUDO=sudo
PERMS=-o root -g root
else
STATIC=yes
endif

include ../Makefile.inc

NAME=esxconf
PROG=$(NAME)$(EXE)
SRCS=$(NAME).c
OBJS=$(SRCS:.c=.o)

all: $(PROG)

#$(OBJS): lib/vim.h
$(OBJS): Makefile

$(PROG): $(OBJS) $(DEPS)
	$(CC) $(CFLAGS) -o $(PROG) $(OBJS) $(LIBS)

install: $(PROG)
ifeq ($(OS),linux)
	$(SUDO) mkdir -p $(BUILDROOT)/usr/local/bin
	$(SUDO) install -m 755 $(PERMS) $(PROG) $(BUILDROOT)/usr/local/bin
endif
#	sudo install -m 755 -o root -g root $(PROG) /var/www/html/tools/$(NAME)_$(ARCH)$(EXE)

clean:
	@rm -f $(PROG) $(OBJS)

cleanall: clean
	@rm -f *.log

rpm:
	sh mkspec
	rpmbuild --buildroot=`pwd`/buildroot -bb builder.spec
#	scp /home/sysadm/rpmbuild/RPMS/noarch/builder-1-0.noarch.rpm ddc-coe:/tmp
#	ssh -t ddc-coe "sudo bash -c \"mv /tmp/builder-1-0.noarch.rpm /coe/misc; /usr/local/bin/sync_coe\""
