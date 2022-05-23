
#DEBUG=yes
ODBC=yes
UTIL=yes

include ../Makefile.inc

#TARGETS=odbc_test socket_client socket_server
TARGETS=odbc_test

all: $(TARGETS)

odbc_test: odbc_test.c
	$(CC) $(CFLAGS) -o odbc_test$(EXE) odbc_test.c $(LIBS)

install:

clean:
	@rm -f $(TARGETS)

cleanall: clean
	@rm -f *.log
