#include "9cc.h"

Node *new_node(int ty, Node *lhs, Node *rhs) {
  Node *node = malloc(sizeof(Node));
  node->ty = ty;
  node->lhs = lhs;
  node->rhs = rhs;
  return node;
}

Node *new_node_num(int val) {
  Node *node = malloc(sizeof(Node));
  node->ty = ND_NUM;
  node->val = val;
  return node;
}


int pos = 0;
int consume(int ty) {
  if (tokens[pos].ty != ty)
    return 0;
  pos++;
  return 1;
}
static void expect(int ty) {
  if(consume(ty)) return;

  char msg[100] = "\0";
  sprintf(msg, "%c expected", (char)ty);
  error_at(tokens[pos].input, msg);
}

void program() {
  int i = 0;
  while(tokens[pos].ty != TK_EOF)
    funcs[i++] = function();
  funcs[i] = NULL;
}

Node *function() {
  Node *node = malloc(sizeof(Node));
  node->ty = ND_FUNC;
  node->args = new_vector();

  if (tokens[pos].ty != TK_IDENT) {
    error_at(tokens[pos].input, "function name expected");
  }
  node->name = tokens[pos++].name;

  expect('(');
  expect(')');
  expect('{');
  node->body = compound_stmt();

  return node;
}

Node *compound_stmt() {
  Node *node = malloc(sizeof(Node));
  node->ty = ND_BLOCK;
  int i = 0;
  while (! consume('}')) {
    node->stmts[i++] = stmt();
  }
  return node;
}
Node *stmt() {
  Node *node;

  if(consume(TK_IF)) {
    expect('(');
    node = malloc(sizeof(Node));
    node->ty = ND_IF;
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
    node->ty = ND_WHILE;
    node->cond = expr();
    expect(')');
    node->body = stmt();
    return node;
  }

  if(consume(TK_FOR)) {
    node = malloc(sizeof(Node));
    node->ty = ND_FOR;

    expect('(');

    // init
    if( tokens[pos].ty == ';' ) {
      node->init = NULL;
      consume(';');
    } else {
      node->init = expr();
      expect(';');
    }

    // cond
    if( tokens[pos].ty == ';' ) {
      node->cond = NULL;
      consume(';');
    } else {
      node->cond = expr();
      expect(';');
    }

    // inc
    if( tokens[pos].ty == ')' ) {
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
    node->ty = ND_BLOCK;

    int i=0;
    while(! consume('}'))
      node->stmts[i++] = stmt();

    return node;
  }

  if (consume(TK_RETURN)) {
    node = malloc(sizeof(Node));
    node->ty = ND_RETURN;
    node->lhs = expr();
  } else {
    node = expr();
  }

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
  return term();
}
Node *term() {
  if(consume('(')) {
    Node *node = expr();
    expect(')');
    return node;
  }

  if (tokens[pos].ty == TK_NUM)
    return new_node_num(tokens[pos++].val);

  if (tokens[pos].ty == TK_IDENT) {
    Node *node = malloc(sizeof(Node));
    node->name = tokens[pos++].name;

    // identifier
    if(!consume('(')) {
      node->ty = ND_IDENT;
      return node;
    }

    // function call
    node->ty = ND_CALL;
    node->args = new_vector();
    if(consume(')'))
      return node;

    vec_push(node->args, (void *)expr());
    while(consume(','))
      vec_push(node->args, (void *)expr());
    expect(')');
    return node;
  }

  error_at(tokens[pos].input, "数値でも識別子でもないトークンです");
}
