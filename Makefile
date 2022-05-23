
all:
	@for d in db soap util vim; do $(MAKE) $(MFLAGS) -C $$d; done
	echo "**** TARGETS ****"
	@for d in *; do test "$$d" = "old" && continue; if test -d $$d; then $(MAKE) $(MFLAGS) -C $$d || exit 1; fi; done

clients:
	@for d in db soap util vim; do $(MAKE) $(MFLAGS) -C $$d; done
	@for d in *; do test -f $$d/.client && $(MAKE) $(MFLAGS) -C $$d install; done
	test -f .vim && sudo install -o root -g root -m 644 .vim /usr/local/etc

install clean cleanall::
	@rm -f *.log
	mkdir -p ~/bin
	for d in *; do if test -d $$d; then $(MAKE) $(MFLAGS) -C $$d $@; fi; done
