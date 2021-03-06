
CC = gcc
CCFLAGS = #-Wall -Werror -O2

.PHONY: all clean cleanall debug install uninstall

all: led
clean:
	rm -f *.o
cleanall: clean
	rm -f led
install:
	cp ./led /usr/local/bin/
uninstall:
	rm -f /usr/local/bin/led

debug: all
	./led

# vim: ts=4 sw=4 sts=0 noet fenc=utf-8

