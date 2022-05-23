
#DEBUG=yes

include ../Makefile.inc

NAME=util
LIB=lib$(NAME)$(SUFFIX).a
SRCS=bindump.c list.c trim.c stredit.c strele.c getpass.c fdutils.c socket.c dns.c encode.c encrypt.c conv.c cfg.c
ifneq ($(OS),win)
SRCS+=getopt.c
endif
OBJS=$(SRCS:.c=.o)

all: $(LIB)

socket.o: socket.h

$(OBJS): util.h Makefile

$(LIB): $(OBJS)
	$(AR) cr $(LIB) $(OBJS)

install: $(LIB)

clean:
	@rm -f $(OBJS)

cleanall: clean
	@rm -f *.a *.log
