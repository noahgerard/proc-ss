#include <stdio.h>
#include <stdbool.h>
#include <malloc.h>
#include <stdlib.h>    // exit()
#include <string.h>    // strcspn(), strtok
#include <errno.h>     // ECHILD

#include "tokens.h"

#include "util.h"

void* xmalloc(size_t size) {
  void *ptr = malloc(size);
  if (ptr == NULL && size != 0) {
    fprintf(stderr, "FATAL: memory exhausted\n");
    exit(1);
  }
  return ptr;
}

char* copystr(char *yytext) {
  size_t len = strlen(yytext);
  char *strval = xmalloc(sizeof(char *) * len + 1);  // add one for the end character
  strval = strncpy(strval, yytext, len);
  strval[len] = '\0';
  return strval;
}

command_t *first_command = NULL;

command_t *last_command = NULL;

command_t *add_command() {
  command_t *new_command = xmalloc(sizeof(command_t));
  for (int i = 0; i < MAX_ARGS; i++) { new_command->argv[i] = NULL; }
  new_command->in = NULL;
  new_command->out = NULL;
  new_command->next = NULL;
  if (NULL == first_command) {
    first_command = last_command = new_command;
  } else {
    last_command->next = new_command;
    last_command = new_command;
  }
  return new_command;
}

void free_commands() {
  command_t *cur = first_command;
  while (NULL != cur) {
    for (int i = 0; i < MAX_ARGS; i++) {
      if (NULL == cur->argv[i]) {
        break;
      }
      free(cur->argv[i]);
    }
    if (NULL != cur->in) {
      free(cur->in);
    }
    if (NULL != cur->out) {
      free(cur->out);
    }
    command_t *next = cur->next;
    free(cur);
    cur = next;
  }
  first_command = NULL;
  last_command = NULL;
}

void print_command(command_t *cur) {
  // print the program with arguments
  for (int i = 0; i < MAX_ARGS; i++) {
    if (NULL == cur->argv[i]) {
      break;
    }
    printf("%s ", cur->argv[i]);
  }
  // print the redirect in if it exists
  if (NULL != cur->in) {
    printf("< %s ", cur->in);
  }
  // print the redirect out if it exists
  if (NULL != cur->out) {
    printf("> %s ", cur->out);
  }
}

void print_commands() {
  command_t *cur = first_command;
  char *delim = "";
  while (NULL != cur) {
    printf("%s", delim);
    print_command(cur);
    cur = cur->next;
    delim = "| ";
  }
  printf("\n");
}

bool match(token_t token1, token_t token2) {
  return token1 == token2;
}

void ensure(token_t token1, token_t token2) {
  if (!match(token1, token2)) {
    error();
  }
}

void error() {
  fprintf(stderr, "parsing error\n");
  exit(EXIT_FAILURE);
}
