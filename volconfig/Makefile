
#DEBUG=yes
ODBC=yes
UTIL=yes

include ../Makefile.inc

NAME=volconfig
PROG=$(NAME)$(EXE)
SRCS=$(NAME).c
OBJS=$(SRCS:.c=.o)

all: $(PROG)

$(OBJS): Makefile

$(PROG): $(OBJS) $(DEPS)
	$(CC) $(CFLAGS) -o $(PROG) $(OBJS) $(LIBS)

install: $(PROG)
ifeq ($(OS),linux)
	sudo install -m 755 -o root -g root $(NAME) /usr/local/sbin
endif
	sudo install -o root -g root -m 755 $(PROG) /var/www/html/tools/$(NAME)_$(ARCH)$(EXE)

install-client:
	$(MAKE) $(MFLAGS) -C scripts install

clean:
	@rm -f $(PROG) $(OBJS)

cleanall: clean
	@rm -f *.log
