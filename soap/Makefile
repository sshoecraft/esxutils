
#DEBUG=yes

include ../Makefile.inc

NAME=soap
LIB=lib$(NAME)$(SUFFIX).a
SRCS=stdsoap2.c soapC.c
OBJS=$(SRCS:.c=.o)

CFLAGS+=-DWITH_OPENSSL -DWITH_DOM -DWITH_COOKIES -DWITH_GZIP

all: $(LIB)

$(OBJS): Makefile

$(LIB): $(OBJS)
	$(AR) cr $(LIB) $(OBJS)

clean:
	@rm -f $(OBJS)

cleanall: clean
	@rm -f *.a *.log
