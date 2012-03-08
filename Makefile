CC=gcc
CFLAGS=-Wall -g
LDFLAGS=-lusb-1.0 -L/lib/x86_64-linux-gnu


all:
	$(CC) $(CFLAGS) $(LDFLAGS) powerusb.c -o powerusb