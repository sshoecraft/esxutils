
#DEBUG=yes
VIM=yes
ODBC=yes
UTIL=yes

include ../Makefile.inc

GENDB_SRCS=gendb_farms.c gendb_hosts.c gendb_datastores.c gendb_host_datastore.c gendb_vms.c gendb_vm_files.c gendb_vm_disks.c gendb_vm_hist.c gendb_host_alarms.c gendb_ci_vw.c

NAME=do_collect
PROG=$(NAME)$(EXE)
SRCS=main.c collect.c farm.c host.c alarm.c vm.c datastore.c hostds.c
ifeq ($(OS),win)
SRCS+=getopt.c
endif
ALL_SRCS=$(SRCS) $(GENDB_SRCS)
OBJS=$(ALL_SRCS:.c=.o)
#CFLAGS+=-DDO_PERF

GENDB=gendb -s esxadmin -u esxadmin_ro

all: $(PROG)

$(OBJS): collect.h Makefile
$(GENDB_SRCS): Makefile

$(PROG): $(OBJS) $(DEPS)
	$(CC) $(CFLAGS) -o $(PROG) $(OBJS) $(LIBS)

gendb_farms.c:
	$(GENDB) -f siu -i id,last_seen -n name farms

gendb_hosts.c:
	 $(GENDB) -f siu -i id,last_seen -n name hosts

gendb_datastores.c:
	 $(GENDB) -f siu -i id,last_seen -n uuid datastores uuid

gendb_host_datastore.c:
	 $(GENDB) host_datastore

#	 $(GENDB) -f siu -i id,last_seen -n name vms
gendb_vms.c:
	 $(GENDB) -f siu -i id,last_seen -n name vms
	 $(GENDB) -f siu -i id,last_seen vms

gendb_vm_files.c:
	 $(GENDB) -f id vm_files

gendb_vm_disks.c:
	 $(GENDB) -f id vm_disks

gendb_vm_hist.c:
	 $(GENDB) -i time vm_hist

gendb_host_alarms.c:
	 $(GENDB) -f fsiu host_alarms host_id

gendb_ci_vw.c:
	 $(GENDB) -f fs ci_vw

$(SRCS): $(GENDB_SRCS)

SCRIPTS=collect alarms clear_alarms
install: $(PROG)
ifeq ($(OS),linux)
	install -s -m 755 $(PROG) $$HOME/bin
	install -m 755 $(SCRIPTS) $$HOME/bin
	for f in $(SCRIPTS); do sed -i -e 's:testing = 1:testing = 0:' $$HOME/bin/$$f; done
endif
#	sudo install -m 755 -o root -g root $(PROG) /var/www/html/tools/$(NAME)_$(ARCH)$(EXE)

clean:
	@rm -f $(PROG) $(OBJS) gendb_*

cleanall: clean
	@rm -f *.log
