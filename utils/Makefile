
#DEBUG=yes

include ../Makefile.inc

NAME=quad2wwn
PROG=$(NAME)$(EXE)
SRCS=$(NAME).c
OBJS=$(SRCS:.c=.o)

all: $(PROG)

#$(OBJS): lib/vim.h
$(OBJS): Makefile

$(PROG): $(OBJS) $(DEPS)
	$(CC) $(CFLAGS) -o $(PROG) $(OBJS) $(LIBS)

install: $(PROG)
ifeq ($(OS),linux)
	sudo install -m 755 -o root -g root $(PROG) /usr/local/bin
endif
	sudo install -m 755 -o root -g root $(PROG) /var/www/html/tools/$(NAME)_$(ARCH)$(EXE)

clean:
	@rm -f $(PROG) $(OBJS)

cleanall: clean
	@rm -f *.log
