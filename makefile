CC=cc
CFLAGS=-Wall -o l
SOURCES=logbook.c utils.c
DBFLAGS=-g -DDB=YES

all:
	$(CC) $(CFLAGS) $(SOURCES)

debug:
	$(CC) $(DBFLAGS) $(CFLAGS) $(SOURCES)

clean:
	rm ./*~
