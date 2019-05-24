#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdnoreturn.h>

void runtest();
noreturn void error(char *fmt, ...);
noreturn void error_at(char *loc, char *msg);
int is_alpha(char c);
int is_digit(char c);
int is_alnum(char c);
// token
enum {
  TK_NUM = 256,
  TK_IDENT,
  TK_RETURN,
  TK_EQ,
  TK_NE,
  TK_LE,
  TK_GE,
  TK_EOF,
};

typedef struct {
  int ty;
  int val;
  char *name; // ty == TK_IDENTのとき、変数名
  char *input;
} Token;

char *user_input;
Token tokens[100];
void tokenize();


//node
enum {
  ND_NUM = 256,
  ND_IDENT,
  ND_RETURN,
};

typedef struct Node {
  int ty;
  struct Node *lhs;
  struct Node *rhs;
  int val;
  char *name; // only use when ty is ND_IDENT
} Node;

Node *code[100];
void program();
Node *stmt();
Node *expr();
Node *assign();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *unary();
Node *term();

void gen(Node *node);



typedef struct {
  void **data;
  int capacity;
  int len;
} Vector;
Vector *new_vector();
void vec_push(Vector *vec, void *elem);

typedef struct {
  Vector *keys;
  Vector *vals;
} Map;
Map *new_map();
void map_put(Map *map, char *key, void *val);
void *map_get(Map *map, char *key);

Map *vars;
int bpoff;
