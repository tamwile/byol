
#include <stdio.h>
#include <stdlib.h>

#include "mpc.h"
#include "evaluation.h"


#ifdef _WIN32

#include <string.h>

static char buffer[2048];

// fake readline function
char* readline(char* prompt) {
  fputs(prompt, stdout);
  fgets(buffer, 2048, stdin);
  char* cpy = malloc(strlen(buffer) + 1);
  strcpy(cpy, buffer);
  cpy[strlen(cpy)-1] = '\0';

  return cpy;
}

// fake history function
void add_history(char* unused) {}

// Otherwise use readline and histedit
#else

#include <editline/readline.h>
#include <histedit.h>

#endif


int main(int argc, char** argv) {

  // Parsers creation
  mpc_parser_t* Number   = mpc_new("number");
  mpc_parser_t* Operator = mpc_new("operator");
  mpc_parser_t* Expr     = mpc_new("expr");
  mpc_parser_t* Lispy    = mpc_new("lispy");

  // Define grammar
  mpca_lang(MPCA_LANG_DEFAULT,
      "                                                    \ 
	number   : /-?[0-9]+/  ;                           \ 
	operator : '+' | '-' | '*' | '/' ;                 \ 
	expr     : <number> | '(' <operator> <expr>+ ')' ; \
	lispy    : /^/ <operator> <expr>+ /$/  ;           \
      ",
      Number, Operator, Expr, Lispy);
  
  puts("Lispy Version 0.0.0.0.1");
  puts("Press Ctrl+c to Exit\n");

  while (1) {

    /* Output our prompt and get input */
    char* input = readline("lispy> ");
    add_history(input);


    // Attempt to parse input
    mpc_result_t r;
    if (mpc_parse("<stdin>", input, Lispy, &r)) {
      mpc_ast_t* a = r.output;
      long result = eval(a);
      printf("%li\n", result);

      mpc_ast_delete(r.output);
    } else {
      // print the error
      mpc_err_print(r.error);
      mpc_err_delete(r.error);
    }
    
    free(input);
  }

  // undefine and delete parsers
  mpc_cleanup(4, Number, Operator, Expr, Lispy);
  
  return 0;
}


int number_of_nodes(mpc_ast_t* t) {
  if (t->children_num == 0) { return 1; }
  if (t->children_num >= 1) {
    int total = 1;
    for (int i = 0; i < t->children_num; i++) {
      total = total + number_of_nodes(t->children[i]);
    }
    return total;
  }
  return 0;
}


long eval_op(long x, char* op, long y) {
  if (strcmp(op, "+") == 0) { return x + y; }
  if (strcmp(op, "-") == 0) { return x - y; }
  if (strcmp(op, "*") == 0) { return x * y; }
  if (strcmp(op, "/") == 0) { return x / y; }
  return 0;
}


long eval(mpc_ast_t* t) {
  // number: base-case => return directly
  if (strstr(t->tag, "number")) {
    return atoi(t->contents);
  }

  // The operator is alway second child
  char* op = t->children[1]->contents;

  // we store the third child in 'x'
  long x = eval(t->children[2]);

  // iterate over the remaining children
  int i = 3;
  while (strstr(t->children[i]->tag, "expr")) {
    x = eval_op(x, op, eval(t->children[i]));
    i++;
  }

  return x;
}



