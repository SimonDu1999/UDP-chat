

CFLAGS = -Wall -g -std=c99 -D _POSIX_C_SOURCE=200809L -Werror -Wno-deprecated-declarations

all: build

build:
	gcc $(CFLAGS) main.c receiver.c printer.c instructorList.o sender.c typer.c shutDownManager.c -lpthread -o s-talk

clean:
	rm -f s-talk
