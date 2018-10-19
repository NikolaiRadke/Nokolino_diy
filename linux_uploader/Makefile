CC=gcc
CFLAGS=-Wall -Os
LDFLAGS=-Wall -s
all: jq6500 jq6500rescue

iso: rescue.iso

jq6500rescue: jq6500.o rescue.o
	$(CC) -no-pie $(LDFLAGS) -o $@ $^

flash: rescue.iso
	./jq6500 -W -r -f -o 0 -s 16 /dev/urandom
	./jq6500 -W -r -o 0 -p rescue.iso

rescue.iso: autorun.inf config.ini jq6500.exe Makefile jq6500 jq6500.c
	mkisofs -o $@ -quiet -V "JQ6500" -appid "JQ6500 Upload Tools" -no-pad $^

%.o: %.iso
	ld --relocatable --format=binary -o $@ $^

clean:
	rm -f jq6500 rescue.iso rescue.o jq6500rescue jq6500.o

.PHONY: all iso clean
