
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

NAME=esxshell
PROG=$(NAME)$(EXE)
SRCS=$(NAME).c
OBJS=$(SRCS:.c=.o)

all: $(PROG)
#	./$(PROG) -s server -x -z config.product

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
