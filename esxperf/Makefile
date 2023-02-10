
#DEBUG=yes
UTIL=yes
VIM=yes
ODBC=yes

include ../Makefile.inc

NAME=get_perf
PROG=$(NAME)$(EXE)
SRCS=main.c getperf.c
OBJS=$(SRCS:.c=.o)

#CFLAGS+=-Wall -Werror

ifeq ($(OS),win)
CFLAGS+=-DWINPERF
endif

#LIBS+=-lodbc

all: $(PROG)

$(OBJS): esxperf.h Makefile

gendb_hosts.c:
	gendb -f siu -i id,last_seen -n name hosts

$(PROG): $(OBJS) $(DEPS)
	$(CC) $(CFLAGS) -o $(PROG) $(OBJS) $(LIBOBJS) $(LIBS) 

install: $(PROG)
ifeq ($(OS),linux)
	install -s -m 755 $(PROG) $$HOME/bin
	install -m 755 esxperf $$HOME/bin
endif
	sudo install -m 755 -o root -g root $(PROG) /var/www/html/tools/$(NAME)_$(ARCH)$(EXE)

clean:
	@rm -f $(PROG) $(OBJS) gendb_*

cleanall:
	@rm -f *.log
