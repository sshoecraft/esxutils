
SRC_ROOT=../..
include ../Makefile.opts
include $(SRC_ROOT)/Makefile.inc

#CFLAGS+=-I. -I..
CFLAGS+=-I..

LIB=libsoapfuncs$(SUFFIX).a
include Makefile.srcs
#OBJS=$(SRCS:.c=.o$(SUFFIX))
OBJS=$(SRCS:.c=.o)

all: $(LIB)

$(OBJS): soapfuncs.h ../Makefile.opts

$(LIB): $(OBJS)
	$(AR) cr $(LIB) $(OBJS)

clean:
	@rm -f $(LIB) $(OBJS)

cleanall: clean
	@rm -f *.a *.log

srcs::
	rm -f Makefile.srcs
	for i in *.c; do echo "SRCS += $$i" >> Makefile.srcs; done
