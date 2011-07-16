CC=cc
CFLAGS=-Wall -o l
SOURCES=logcli.c utils.c
DBFLAGS=-g -DDB=YES

all:
	$(CC) $(CFLAGS) $(SOURCES)

debug:
	$(CC) $(DBFLAGS) $(CFLAGS) $(SOURCES)

clean:
	rm ./*~
