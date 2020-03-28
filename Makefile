CC=gcc
CFLAGS=	-Wall	-Werror	-g#-O2
LDFLAGS=
INCLUDE=

SRC=urlextendmain.c urlextend.c
BIN=urlextend
OBJ=$(SRC:.c=.o)

INCLUDES=urlextend.h

$(BIN):	$(OBJ) $(INCLUDES)
	$(CC) $(CFLAGS) -o $(BIN)	$(OBJ)	$(LDFLAGS)


.PHONY : clean


clean:
	rm	$(OBJ)	
	rm	$(BIN)
