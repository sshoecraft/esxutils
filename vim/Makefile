
#opts are in Makefile.opts
include Makefile.opts
include ../Makefile.inc

LIB=libvim$(SUFFIX).a
SRCS=session.c error.c getinfo.c getresult.c cred.c utils.c perf.c task.c res2desc.c host.c folder.c mm.c misc.c getvc.c
#OBJS=$(SRCS:.c=.o$(SUFFIX))
OBJS=$(SRCS:.c=.o)
SFLIB=soapfuncs/libsoapfuncs$(SUFFIX).a
CFLAGS+=-Isoapfuncs

all: $(LIB)
#all: $(LIB) test
#	./test

test: test.c $(LIB) $(DEPS)
	$(CC) $(CFLAGS) -o $@ $< $(LIB) $(LIBS)

$(OBJS): Makefile.opts vim.h

$(SFLIB)::
	$(MAKE) $(MFLAGS) -C soapfuncs

$(LIB): $(OBJS) $(SFLIB)
	$(AR) cr $(LIB) $(OBJS) soapfuncs/*.o

install: $(LIB)

clean:
	@rm -f $(OBJS) test
	@$(MAKE) $(MFLAGS) -C soapfuncs clean

cleanall: clean
	@rm -f *.a *.log
	@$(MAKE) $(MFLAGS) -C soapfuncs cleanall
