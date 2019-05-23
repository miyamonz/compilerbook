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

void program() {
  int i = 0;
  while (tokens[pos].ty != TK_EOF)
    code[i++] = stmt();
  code[i] = NULL;
}
Node *stmt() {
  Node *node = expr();
  if (!consume(';'))
    error_at(tokens[pos].input, "';'ではないトークンです");
  return node;
}
Node *expr() {
  return equality();
}

Node *equality() {
  Node *node = relational();
  for (;;) {
    if (consume(TK_EQ))
      node = new_node(TK_EQ, node, relational());
    else if (consume(TK_NE))
      node = new_node(TK_NE, node, relational());
    else
      return node;
  }
}

Node *relational() {
  Node *node = add();
  for (;;) {
    if (consume(TK_LE))
      node = new_node(TK_LE, node, add());
    else if (consume(TK_GE))
      node = new_node(TK_LE, add(), node);
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

    if(!consume(')'))
      error_at(tokens[pos].input, "対応する閉じカッコがありません");
    return node;
  }

  return num();
}

Node *num() {
  if (tokens[pos].ty == TK_NUM)
    return new_node_num(tokens[pos++].val);
  error_at(tokens[pos].input, "数値でないトークンです");
}
