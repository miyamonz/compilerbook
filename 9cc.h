#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdnoreturn.h>

noreturn void error(char *fmt, ...);
noreturn void error_at(char *loc, char *msg);
// token
enum {
  TK_NUM = 256,
  TK_EQ,
  TK_NE,
  TK_LE,
  TK_GE,
  TK_EOF,
};

typedef struct {
  int ty;
  int val;
  char *input;
} Token;

char *user_input;
Token tokens[100];
void tokenize();


//node
enum {
  ND_NUM = 256,
};

typedef struct Node {
  int ty;
  struct Node *lhs;
  struct Node *rhs;
  int val;
} Node;

Node *expr();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *unary();
Node *term();
Node *num();

void gen(Node *node);



typedef struct {
  void **data;
  int capacity;
  int len;
} Vector;
Vector *new_vector();
void vec_push(Vector *vec, void *elem);
