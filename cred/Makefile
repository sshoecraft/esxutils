
#DEBUG=yes
UTIL=yes
VIM=yes

include ../Makefile.inc

NAME=vim_cred
PROG=$(NAME)$(EXE)
SRCS=main.c
OBJS=$(SRCS:.c=.o)
#ifeq ($(OS),win)
#LIBS+=-luserenv
#endif
#LIBS+=-lcrypt

all: $(PROG)
#	./$(PROG) -a -s try -u this -p one
#	./vim_cred -e -s g1t0698h -u root

$(PROG): $(OBJS) $(DEPS)
	$(CC) $(CFLAGS) -o $(PROG) $(OBJS) $(LIBS) 

$(OBJS): Makefile

install: $(PROG)
ifeq ($(OS),linux)
	sudo install -m 755 -o root -g root $(PROG) /usr/local/bin
endif
	sudo install -o root -g root -m 755 $(PROG) /var/www/html/tools/$(NAME)_$(ARCH)$(EXE)

clean:
	@rm -f $(PROG) $(OBJS)

cleanall: clean
	@rm -f *.log
