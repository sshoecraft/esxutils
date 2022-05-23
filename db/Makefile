
#DEBUG=yes

include ../Makefile.inc

NAME=db
LIB=lib$(NAME)$(SUFFIX).a
SRCS=err.c connect.c exec.c fetch.c data.c
OBJS=$(SRCS:.c=.o)

all: $(LIB)

$(OBJS): Makefile

$(LIB): $(OBJS)
	$(AR) cr $(LIB) $(OBJS)

install: $(LIB)

clean:
	@rm -f $(OBJS)

cleanall: clean
	@rm -f *.a *.log
