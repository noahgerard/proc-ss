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
  printf("commands all run, next\n");
}

void close_out_end(int pipefd[2]) {
  if (-1 == close(pipefd[0])) {
    perror("close");
    exit(EXIT_FAILURE);
  }
}
void close_in_end(int pipefd[2]) {
  if (-1 == close(pipefd[1])) {
    perror("close");
    exit(EXIT_FAILURE);
  }
}
void wait_till_children_die() {
  do {
    while (wait(NULL) > 0)
      ;
  } while (errno != ECHILD);
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

      // Input redirection
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

      // Output redirection
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
    wait_till_children_die();
  } else if (NULL != first_command) {
    command_t *command = first_command;
    command_t *prev_command = NULL;
    int pipefd[2];
    int prev_pipe_out = -1;

    while (NULL != command) {
      // Open pipe and store fd's in pipefd
      if (command->pipe_next) {
        if (-1 == pipe(pipefd)) {
          perror("pipe");
          exit(EXIT_FAILURE);
        }
      }

      pid_t pid;
      switch (pid = fork()) {
      case -1:
        perror("fork");
        exit(EXIT_FAILURE);
        break;
      case 0: // child
        if (prev_pipe_out != -1) {
          if (-1 == dup2(prev_pipe_out, STDIN_FILENO)) {
            perror("dup2");
            exit(EXIT_FAILURE);
          }
          if (-1 == close(prev_pipe_out)) {
            perror("close");
            exit(EXIT_FAILURE);
          }
        }

        if (command->pipe_next) {
          if (-1 == close(pipefd[0])) {
            perror("close");
            exit(EXIT_FAILURE);
          }
          if (-1 == dup2(pipefd[1], STDOUT_FILENO)) {
            perror("dup2");
            exit(EXIT_FAILURE);
          }
          if (-1 == close(pipefd[1])) {
            perror("close");
            exit(EXIT_FAILURE);
          }
        }

        execvp(command->argv[0], command->argv);
        perror("execvp");
        _exit(EXIT_FAILURE);
        break;
      default: // parent
        if (prev_pipe_out != -1) {
          close(prev_pipe_out);
          prev_pipe_out = -1; // Reset it
        }

        // 2. If we just created a NEW pipe for the current child to write into:
        if (command->pipe_next) {
          // We MUST close the write end in the parent.
          // Only the child should be writing to this.
          close_in_end(pipefd);

          // Now, save the read end as the baton for the NEXT loop.
          prev_pipe_out = pipefd[0];
        }

        command = command->next;
        break;
      }
    }

    // Close pipe fds
    wait_till_children_die();
  }
}
