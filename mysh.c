#include <errno.h>    // ECHILD
#include <fcntl.h>    // O_RDONLY, open
#include <inttypes.h> // intmax_t
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h> // exit()
#include <string.h> // strcspn(), strtok
#include <sys/fcntl.h>
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
void parse() {
  consume();

  // TODO: validate start and pipes
start:
pipes:

command:
  ensure(lookahead, STRING);
  command_t *command = add_command(); // create new command list entry

program:
  ensure(lookahead, STRING);
  command->argv[0] = lexeme; // populate data in the list entry
  consume();
  // Collect arguments
  int arg_count = 1;
  while (match(lookahead, STRING)) {
    command->argv[arg_count] = lexeme;
    arg_count++;
    consume();
  }

in:
  if (match(lookahead, REDIRECT_IN)) {
    consume();
    ensure(lookahead, STRING);
    command->in = lexeme;
    consume();
  }

out:
  if (match(lookahead, REDIRECT_OUT)) {
    consume();
    ensure(lookahead, STRING);
    command->out = lexeme;
    consume();
  }

  if (match(lookahead, PIPE)) {
    command->pipe_next = true;
    consume();
    goto pipes;
  }

  ensure(lookahead, END_OF_LINE);

  // TODO: Be sure to remove any debugging output to stdout before submitting
  printf("PRINTING COMMANDS!\n");
  print_commands();
}

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

void run_commands() {
  if (NULL != first_command && first_command == last_command) {
    // one command in the list

    command_t *command = first_command;

    pid_t pid;
    switch (pid = fork()) {
    case -1:
      perror("fork");
      exit(EXIT_FAILURE);
      break;
    case 0: // child

      if (NULL != command->in) {
        int infd = open(command->in, O_RDONLY);
        if (-1 == infd) {
          perror("open");
          exit(EXIT_FAILURE);
        }
        if (-1 == dup2(infd, STDIN_FILENO)) {
          perror("dup2");
          exit(EXIT_FAILURE);
        }
        if (-1 == close(infd)) {
          perror("close");
          exit(EXIT_FAILURE);
        }
      }

      if (NULL != command->out) {
        int outfd =
            open(command->out, O_RDWR | O_TRUNC | O_CREAT, S_IRUSR | S_IWUSR);
        if (-1 == outfd) {
          perror("open");
          exit(EXIT_FAILURE);
        }
        if (-1 == dup2(outfd, STDOUT_FILENO)) {
          perror("dup2");
          exit(EXIT_FAILURE);
        }
        if (-1 == close(outfd)) {
          perror("close");
          exit(EXIT_FAILURE);
        }
      }

      execvp(command->argv[0], command->argv);
      perror("execvp");
      _exit(EXIT_FAILURE);
      break;
    default: // parent
      // nothing to do here.  will wait for all children once all child
      // processes are created.
      break;
    }

    // wait for children to exit
    do {
      while (wait(NULL) > 0)
        ;
    } while (errno != ECHILD);
  }
}
