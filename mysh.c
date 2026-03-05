#include <errno.h>    // ECHILD
#include <fcntl.h>    // O_RDONLY, open
#include <inttypes.h> // intmax_t
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>   // exit()
#include <string.h>   // strcspn(), strtok
#include <sys/wait.h> // wait()
#include <unistd.h>   // fork()

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

void parse() {}

void run_commands() {}
