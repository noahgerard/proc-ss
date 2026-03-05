#include <stdio.h>
#include <stdbool.h>
#include <malloc.h>
#include <unistd.h>    // fork()
#include <stdlib.h>    // exit()
#include <inttypes.h>  // intmax_t
#include <sys/wait.h>  // wait()
#include <string.h>    // strcspn(), strtok
#include <errno.h>     // ECHILD
#include <fcntl.h>     // O_RDONLY, open

#include "tokens.h"

#include "util.h"

#include "lex.c"

/* Parser lookahead */
token_t lookahead;

/* The lookahead's lexeme */
char *lexeme;

/* Consume one token from the lexer. */
void consume();

/* Command-line parser. */
void parse();

/* Command interpreter. */
void run_commands();

void consume() {
  lookahead = yylex();
  lexeme = copystr(yytext);
}

int main(int argc, char **argv) {
  parse();
  run_commands();
}

void parse() {
}

void run_commands() {
}
