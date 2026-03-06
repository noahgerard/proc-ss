#ifndef UTIL_H
#define UTIL_H

/* malloc or die.  Adapted from busybox's xfuncs_printf.c */
void *xmalloc(size_t size);

/*
 * Make a copy of the lexer's text value, since it will use that
 * buffer for other things.
 */
char *copystr(char *yytext);

/*
 * Only required to support up to 62 arguments, 64 when including the
 * program name and the NULL terminator array entry.  The real system
 * defines ARG_MAX in linux/limits.h.  This actual limit could be
 * exceeded even with MAX_ARGS below, because this program does not
 * control the total size of the argv parameter to exec.
 */
#define MAX_ARGS 64

/*
 * This struct holds shell commands, potentially a list of them for
 * pipes.  If there is only one command, then there are no pipes.
 */
typedef struct command_s {
  // NULL-terminated list, i.e., the next element after the last should be NULL.
  char *argv[MAX_ARGS];
  char *in;       // NULL if there is no redirect
  char *out;      // NULL if there is no redirect
  bool pipe_next; // Should pipe output to next command?
  struct command_s *next;
} command_t;

/* The head of the list of piped commands. */
extern command_t *first_command;

/* The tail of the list of piped commands. */
extern command_t *last_command;

/*
 * A helper function that both creates a new entry in the list of
 * piped commands and adds it to the list of piped commands.  It
 * returns a pointer to newly-created command struct with all elements
 * set to NULL.
 */
command_t *add_command();

/*
 * Free memory and reset the command list.
 */
void free_commands();

/*
 * A helper function to pretty print the command_s data structure and
 * list.  This is useful for debugging the parser.
 */
void print_commands();

/*
 * A helper function to pretty print one element in a command_s list.
 * This is useful for debugging the parser.
 */
void print_command(command_t *);

/* Check that two tokens match when parsing. */
bool match(token_t, token_t);

/* Error if two tokens don't match when parsing. */
void ensure(token_t, token_t);

/* Error and quit for parser errors. */
void error();

#endif
