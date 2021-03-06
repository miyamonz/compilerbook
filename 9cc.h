#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdnoreturn.h>
#include <stdint.h>

// type
typedef struct Type {
  enum { INT, PTR, ARRAY } ty;
  struct Type *ptr_to;
  size_t array_size;
} Type;

// util.c
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

typedef struct LVar LVar;
struct LVar {
  LVar *next;
  char *name;
  int len;
  int offset;
  Type *ty;
};
LVar *locals;

void runtest();
noreturn void error(char *fmt, ...);
noreturn void error_at(char *loc, char *msg, ...);
int is_alpha(char c);
int is_digit(char c);
int is_alnum(char c);
// token
typedef enum {
  TK_NUM = 256,
  TK_IDENT,
  TK_INT,
  TK_SIZEOF,
  TK_RETURN,
  TK_IF,
  TK_ELSE,
  TK_WHILE,
  TK_FOR,
  TK_EQ, // ==
  TK_NE, // !=
  TK_LE, // <=
  TK_GE, // >=
  TK_EOF,
} TokenKind;

typedef struct Token Token;
struct Token {
  TokenKind kind;
  Token *next;
  int val; // kind がTK_NUMのときの数値
  char *name; // kind == TK_IDENTのとき、変数名
  char *str; //トークン文字列
  int len; //トークン長さ
};

char *user_input;
//現在着目しているトークン
Token *token;
Token *tokenize(char *);


//node
enum {
  ND_NUM = 256,
  ND_IDENT,
  ND_VARDEF,
  ND_CALL,
  ND_FUNC,
  ND_RETURN,
  ND_IF,
  ND_WHILE,
  ND_FOR,
  ND_BLOCK,
  ND_EQ,
  ND_NE,
  ND_LE,
  ND_DEREF,
  ND_ADDR,
};

typedef struct Node {
  int op;
  struct Node *lhs;
  struct Node *rhs;
  int val;

  // ND_CALL, ND_FUNC
  char *name;

  // ND_IDENT, ND_VARDEF, ND_ADDR
  LVar *var;

  Type *ty;

  //if
  struct Node *cond;
  struct Node *then;
  struct Node *els;

  // while (cond) body;
  struct Node *body;

  //for(init; cond; inc) body;
  struct Node *init;
  struct Node *inc;

  //block { stmt* }
  struct Node *stmts[200];

  // function call
  Vector *args;
  LVar *locals;

  //debug
  char *str;
} Node;

Node *funcs[100];
void program();

void gen(Node *node);
void gen_func(Node *node);
int label;
