CC=gcc
CFLAGS=-Wall -Os -s
all: jq6500
iso: jq6500.iso

flash: jq6500.iso
	./jq6500 -W -r -f -o 0 -s 16 /dev/urandom
	./jq6500 -W -r -o 0 -p jq6500.iso

jq6500.iso: autorun.inf config.ini jq6500.exe Makefile jq6500 jq6500.c
	mkisofs -o $@ -quiet -V "JQ6500" -appid "JQ6500 Upload Tools" -no-pad $^

clean:
	rm -f jq6500 jq6500.iso

.PHONY: all iso clean