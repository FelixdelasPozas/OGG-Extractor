# Makefile for Generic Ogg Ripper
SRC=main.c
OBJ=$(SRC:.c=.o) # replaces the .c from SRC with .o
EXE=OggRipper.exe

CC=gcc
CFLAGS=-Wall -Os -s
LDFLAGS = 
RM=del

%.o: %.c         # combined w/ next line will compile recently changed .c files
	$(CC) $(CFLAGS) -o $@ -c $<

$(EXE): $(OBJ)   # $(EXE) is dependent on all of the files in $(OBJ) to exist
	$(CC) $(CFLAGS) $(OBJ) $(LDFLAGS) -o $@ 

.PHONY : clean   # .PHONY ignores files named clean
clean:
	-$(RM) $(OBJ)


