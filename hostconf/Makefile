
all:

install:
	sudo install -m 755 -o root -g root hostconf /usr/local/bin
	sudo sed -i -e 's:testing=1:testing=0:' /usr/local/bin/hostconf

clean cleanall:
