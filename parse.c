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

Node *new_node(int op, Node *lhs, Node *rhs, Type *ty) {
  Node *node = malloc(sizeof(Node));
  node->op = op;
  node->lhs = lhs;
  node->rhs = rhs;
  node->ty = ty;
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
  Node *m = mul();
  return new_node(ND_DEREF, m, NULL, m->ty->ptr_to);
}

static Type *ptr_of(Type *base);
Token *consume_ident();
LVar *find_lvar(Token *tok);
Node *new_node_addr() {
  Token *tok = consume_ident();
  LVar *lvar = find_lvar(tok);
  if(lvar == NULL)
    error_at(tok->str, "variable is not defined");

  Node *node = new_node(ND_ADDR, NULL, NULL, ptr_of(lvar->ty));
  node->name = lvar->name;
  node->offset = lvar->offset;
  return node;
}

// util
int consume(int ty) {
  if (token->kind != ty)
    return 0;
  token = token->next;
  return 1;
}

Token *consume_ident() {
  if( token->kind != TK_IDENT )
    error_at(token->str, "ident expected.");
  Token *t = token;
  token = token->next;
  return t;
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
  ty->ptr_to = base;
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

//variable
LVar *find_lvar(Token *tok) {
  for(LVar *var = locals; var; var = var->next) {
    if(var->len == tok->len && !memcmp(tok->str, var->name, var->len))
      return var;
  }
  return NULL;
}
LVar *put_lvar(Token *tok, Type* ty) {
  LVar *lvar = calloc(1, sizeof(LVar));
  lvar->next = locals;

  lvar->name = tok->name;
  lvar->len = tok->len;
  lvar->offset = locals->offset + 8;
  lvar->ty = ty;

  locals = lvar;
  return lvar;
}

void program() {
  int i = 0;
  while(token->kind != TK_EOF) {
    locals = calloc(1, sizeof(LVar));
    locals->offset = 0;
    funcs[i++] = function();
  }
  funcs[i] = NULL;
}

Node *function() {
  Node *node = malloc(sizeof(Node));
  node->op = ND_FUNC;
  node->args = new_vector();

  expect(TK_INT);

  Token *tok = consume_ident();
  node->name = tok->name;

  expect('(');
  if(! consume(')')) {
    vec_push(node->args, (void *)decl());
    while(consume(','))
      vec_push(node->args, (void *)decl());
    expect(')');
  }
  expect('{');
  node->body = compound_stmt();

  node->locals = locals;

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

  if(token->kind == TK_INT) {
    Node *d = decl();
    expect(';');
    return d;
  }

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

  Token *tok = consume_ident();
  node->name = tok->name;

  LVar *lvar = find_lvar(tok);
  if (lvar)
    error_at(token->str, "variable %s is already defined.", tok->name);

  // create new variable
  lvar = put_lvar(tok, node->ty);
  node->offset = lvar->offset;

  return node;
}

Node *expr() {
  return assign();
}

Node *assign() {
  Node *node = equality();
  if(consume('='))
    return new_node('=', node, assign(), &int_ty);
  return node;

}

Node *equality() {
  Node *node = relational();
  for (;;) {
    if (consume(TK_EQ))
      node = new_node(ND_EQ, node, relational(), &int_ty);
    else if (consume(TK_NE))
      node = new_node(ND_NE, node, relational(), &int_ty);
    else
      return node;
  }
}

Node *relational() {
  Node *node = add();
  for (;;) {
    if (consume(TK_LE))
      node = new_node(ND_LE, node, add(), &int_ty);
    else if (consume(TK_GE))
      node = new_node(ND_LE, add(), node, &int_ty);
    else if (consume('<'))
      node = new_node('<', node, add(), &int_ty);
    else if (consume('>'))
      node = new_node('<', add(), node, &int_ty);
    else
      return node;
  }
}

void swap(Node **p, Node ** q) {
  Node *r = *p;
  *p = *q;
  *q = r;
}
Node *add() {
  Node *node = mul(); 

  for (;;) {
    // TODO: check pointer and change type
    if (consume('+')) {
      Node *rhs = mul();
      if(node->ty->ty == PTR && rhs->ty->ty == PTR)
        error_at(token->str, "pointer + pointer is invalid.");
      if(rhs->ty->ty == PTR) swap(&rhs, &node);
      node = new_node('+', node, rhs, node->ty);
    } else if (consume('-')) {
      Node *rhs = mul();
      if(node->ty->ty == PTR && rhs->ty->ty == PTR)
        error_at(token->str, "pointer - pointer is invalid.");
      if(rhs->ty->ty == PTR) swap(&rhs, &node);

      node = new_node('-', node, rhs, node->ty);
    } else {
      return node;
    }
  }
}

Node *mul() {
  Node *node = unary();

  for (;;) {
    if (consume('*'))
      node = new_node('*', node, unary(), node->ty);
    else if (consume('/'))
      node = new_node('/', node, unary(), node->ty);
    else
      return node;
  }
}

Node *unary() {
  if(consume(TK_SIZEOF)) {
    Node *n = unary();
    return new_node_num(n->ty->ty == PTR ? 8 : 4);
  }

  if(consume('+'))
    return term();
  if(consume('-'))
    return new_node('-', new_node_num(0), term(), &int_ty);
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
    Token *tok = consume_ident();
    node->name = tok->name;

    // identifier
    if(!consume('(')) {
      node->op = ND_IDENT;

      LVar *lvar = find_lvar(tok);
      if (lvar == NULL)
        error_at(token->str, "variable not defined yet.");

      node->offset = lvar->offset;
      node->ty = lvar->ty;
      node->name = lvar->name;
      return node;
    }

    // function call
    node->op = ND_CALL;
    node->args = new_vector();
    node->ty = &int_ty; // currently only support return int
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
