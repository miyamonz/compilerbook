#include "9cc.h"

// 前方宣言
Node *function();
Node *compound_stmt();
Node *stmt();
Node *decl();
Node *expr();
Node *assign();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *unary();
Node *term();

static Type int_ty = {INT, NULL};

Node *new_node(int op, Node *lhs, Node *rhs) {
  Node *node = malloc(sizeof(Node));
  node->op = op;
  node->lhs = lhs;
  node->rhs = rhs;
  return node;
}

Node *new_node_num(int val) {
  Node *node = malloc(sizeof(Node));
  node->op = ND_NUM;
  node->ty = &int_ty;
  node->val = val;
  return node;
}
Node *new_node_deref() {
  return new_node(ND_DEREF, mul(), NULL);
}
static Type *ptr_of(Type *base);
Node *new_node_addr() {
  return new_node(ND_ADDR, mul(), NULL);
}

// util
int consume(int ty) {
  if (token->kind != ty)
    return 0;
  token = token->next;
  return 1;
}
static void expect(int ty) {
  if(consume(ty)) return;

  char msg[100] = "\0";
  sprintf(msg, "%c expected", (char)ty);
  error_at(token->str, msg);
}
static int expect_number() {
  if(token->kind != TK_NUM)
    error_at(token->str, "数ではありません");
  int val = token->val;
  token = token->next;
  return val;
}

static Type *ptr_of(Type *base) {
  Type *ty = malloc(sizeof(Type));
  ty->ty = PTR;
  ty->ptrof = base;
  return ty;
}
static Type *type() {
  if(!consume(TK_INT))
    error_at(token->str, "int expected");

  Type *ty = &int_ty;
  while(consume('*'))
    ty = ptr_of(ty);
  return ty;
}

void program() {
  int i = 0;
  while(token->kind != TK_EOF)
    funcs[i++] = function();
  funcs[i] = NULL;
}

Node *param() {
  Node *node = malloc(sizeof(Node));
  node->op = ND_VARDEF;
  node->ty = type();

  if( token->kind != TK_IDENT )
    error_at(token->str, "型の後には変数名が必要です");
  node->name = token->name;
  token = token->next;

  return node;
}
Node *function() {
  Node *node = malloc(sizeof(Node));
  node->op = ND_FUNC;
  node->args = new_vector();

  expect(TK_INT);

  if (token->kind != TK_IDENT) {
    error_at(token->str, "function name expected");
  }
  node->name = token->name;
  token = token->next;

  expect('(');
  if(! consume(')')) {
    vec_push(node->args, (void *)param());
    while(consume(','))
      vec_push(node->args, (void *)param());
    expect(')');
  }
  expect('{');
  node->body = compound_stmt();

  return node;
}

Node *compound_stmt() {
  Node *node = malloc(sizeof(Node));
  node->op = ND_BLOCK;
  int i = 0;
  while (! consume('}')) {
    node->stmts[i++] = stmt();
  }
  return node;
}
Node *stmt() {
  Node *node;

  if(token->kind == TK_INT)
    return decl();

  if(consume(TK_IF)) {
    expect('(');
    node = malloc(sizeof(Node));
    node->op = ND_IF;
    node->cond = expr();
    expect(')');
    node->then = stmt();
    if(consume(TK_ELSE))
       node->els = stmt();
    return node;
  }

  if(consume(TK_WHILE)) {
    expect('(');
    node = malloc(sizeof(Node));
    node->op = ND_WHILE;
    node->cond = expr();
    expect(')');
    node->body = stmt();
    return node;
  }

  if(consume(TK_FOR)) {
    node = malloc(sizeof(Node));
    node->op = ND_FOR;

    expect('(');

    // init
    if( token->kind == ';' ) {
      node->init = NULL;
      consume(';');
    } else {
      node->init = expr();
      expect(';');
    }

    // cond
    if( token->kind == ';' ) {
      node->cond = NULL;
      consume(';');
    } else {
      node->cond = expr();
      expect(';');
    }

    // inc
    if( token->kind == ')' ) {
      node->inc = NULL;
      consume(')');
    } else {
      node->inc = expr();
      expect(')');
    }

    node->body = stmt();
    return node;
  }

  if (consume('{')) {
    node = malloc(sizeof(Node));
    node->op = ND_BLOCK;

    int i=0;
    while(! consume('}'))
      node->stmts[i++] = stmt();

    return node;
  }

  if (consume(TK_RETURN)) {
    node = malloc(sizeof(Node));
    node->op = ND_RETURN;
    node->lhs = expr();
  } else {
    node = expr();
  }

  expect(';');

  return node;
}

Node *decl() {
  Node *node = malloc(sizeof(Node));
  node->op = ND_VARDEF;
  node->ty = type();

  if( token->kind != TK_IDENT )
    error_at(token->str, "型の後には変数名が必要です");
  node->name = token->name;
  token = token->next;

  expect(';');
  return node;
}

Node *expr() {
  return assign();
}

Node *assign() {
  Node *node = equality();
  if(consume('='))
    return new_node('=', node, assign());
  return node;

}

Node *equality() {
  Node *node = relational();
  for (;;) {
    if (consume(TK_EQ))
      node = new_node(ND_EQ, node, relational());
    else if (consume(TK_NE))
      node = new_node(ND_NE, node, relational());
    else
      return node;
  }
}

Node *relational() {
  Node *node = add();
  for (;;) {
    if (consume(TK_LE))
      node = new_node(ND_LE, node, add());
    else if (consume(TK_GE))
      node = new_node(ND_LE, add(), node);
    else if (consume('<'))
      node = new_node('<', node, add());
    else if (consume('>'))
      node = new_node('<', add(), node);
    else
      return node;
  }
}
Node *add() {
  Node *node = mul(); 

  for (;;) {
    if (consume('+'))
      node = new_node('+', node, mul());
    else if (consume('-'))
      node = new_node('-', node, mul());
    else
      return node;
  }
}

Node *mul() {
  Node *node = unary();

  for (;;) {
    if (consume('*'))
      node = new_node('*', node, unary());
    else if (consume('/'))
      node = new_node('/', node, unary());
    else
      return node;
  }
}

Node *unary() {
  if(consume('+'))
    return term();
  if(consume('-'))
    return new_node('-', new_node_num(0), term());
  if(consume('*'))
    return new_node_deref();
  if(consume('&'))
    return new_node_addr();
  return term();
}
Node *term() {
  if(consume('(')) {
    Node *node = expr();
    expect(')');
    return node;
  }

  if (token->kind == TK_NUM)
    return new_node_num(expect_number());

  if (token->kind == TK_IDENT) {
    Node *node = malloc(sizeof(Node));
    node->name = token->name;
    token = token->next;


    // identifier
    if(!consume('(')) {
      node->op = ND_IDENT;
      return node;
    }

    // function call
    node->op = ND_CALL;
    node->args = new_vector();
    if(consume(')'))
      return node;

    vec_push(node->args, (void *)expr());
    while(consume(','))
      vec_push(node->args, (void *)expr());
    expect(')');
    return node;
  }

  error_at(token->str, "数値でも識別子でもないトークンです");
}
