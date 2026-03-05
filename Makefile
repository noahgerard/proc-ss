PROG := mysh
SRC = mysh.c util.c
GEN = lex.c
OBJ = $(SRC:%.c=%.o)

.PHONY: all clean

all: $(PROG)

lex.c: lex.l
	flex -o $@ $<

mysh.o: mysh.c lex.c

$(PROG): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	$(RM) $(PROG) $(OBJ) $(GEN)

PUBLISH_DIR := /home/bpappas/documents/teaching/cop3402/spr2026/org/files

starter:
	tar -cvf $(PUBLISH_DIR)/proc.tar lex.l Makefile mysh.template.c tokens.h util.c util.h
