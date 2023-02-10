
#DEBUG=yes
VIM=yes
ODBC=yes
UTIL=yes

include ../Makefile.inc

GENDB_SRCS=gendb_farms.c gendb_vms.c

NAME=destroy_vm
PROG=$(NAME)$(EXE)
SRCS=destroy_vm.c
ifeq ($(OS),win)
SRCS+=getopt.c
endif
ALL_SRCS=$(SRCS) $(GENDB_SRCS)
OBJS=$(ALL_SRCS:.c=.o)
#CFLAGS+=-DDO_PERF

GENDB=gendb -s esxadmin -u esxadmin_ro -p esxadmin_ro

all: $(PROG)

$(OBJS): Makefile
$(GENDB_SRCS): Makefile

$(PROG): $(OBJS) $(DEPS)
	$(CC) $(CFLAGS) -o $(PROG) $(OBJS) $(LIBS)

#	$(GENDB) -f siu -i id,last_seen -n name farms
gendb_farms.c:
	$(GENDB) -f siu -i id -n name farms

#	 $(GENDB) -f siu -i id,last_seen -n name vms
gendb_vms.c:
	 $(GENDB) -f siu -i id -n name vms

$(SRCS): $(GENDB_SRCS)

SCRIPTS=collect alarms clear_alarms
install: $(PROG)
ifeq ($(OS),linux)
	sudo install -s -m 755 $(PROG) /usr/local/bin/
endif
#	sudo install -m 755 -o root -g root $(PROG) /var/www/html/tools/$(NAME)_$(ARCH)$(EXE)

clean:
	@rm -f $(PROG) $(OBJS) gendb_*
