
CC=gcc
CFLAGS=-I .
# -fsanitize=address

all: Music_App
SOURCES= sonic.c sonic.h Equalizer.c Equalizer.h FirFilter.c FirFilter.h

Music_App: Music_App.c
	$(CC) $(CFLAGS) -o Music_App Music_App.c $(SOURCES) -lasound -pthread -lm

clean:
	$(RM) Music_App