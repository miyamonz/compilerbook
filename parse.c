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
  Node *node;

  if(consume(TK_IF)) {
    if(!consume('('))
      error_at(tokens[pos].input, "ifにカッコがありません\n");
    node = malloc(sizeof(Node));
    node->ty = ND_IF;
    node->cond = expr();
    if(!consume(')'))
      error_at(tokens[pos].input, "if(...に対応する閉じカッコがありません\n");
    node->then = stmt();
    if(consume(TK_ELSE))
       node->els = stmt();
    return node;
  }

  if(consume(TK_WHILE)) {
    if(!consume('('))
      error_at(tokens[pos].input, "whileにカッコがありません\n");
    node = malloc(sizeof(Node));
    node->ty = ND_WHILE;
    node->cond = expr();
    if(!consume(')'))
      error_at(tokens[pos].input, "while(...に対応する閉じカッコがありません\n");
    node->body = stmt();
    return node;
  }

  if(consume(TK_FOR)) {
    node = malloc(sizeof(Node));
    node->ty = ND_FOR;

    if(!consume('('))
      error_at(tokens[pos].input, "forにカッコがありません\n");

    // init
    if( tokens[pos].ty == ';' ) {
      node->init = NULL;
      consume(';');
    } else {
      node->init = expr();
      if(!consume(';'))
        error_at(tokens[pos].input, ";がありません\n");
    }

    // cond
    if( tokens[pos].ty == ';' ) {
      node->cond = NULL;
      consume(';');
    } else {
      node->cond = expr();
      if(!consume(';'))
        error_at(tokens[pos].input, ";がありません\n");
    }

    // inc
    if( tokens[pos].ty == ')' ) {
      node->inc = NULL;
      consume(')');
    } else {
      node->inc = expr();
      if(!consume(')'))
        error_at(tokens[pos].input, ")がありません\n");
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

  if (!consume(';'))
    error_at(tokens[pos].input, "';'ではないトークンです");
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

  if (tokens[pos].ty == TK_NUM)
    return new_node_num(tokens[pos++].val);

  if (tokens[pos].ty == TK_IDENT) {
    Node *node = malloc(sizeof(Node));
    node->ty = ND_IDENT;
    node->name = tokens[pos++].name;
    return node;
  }
  error_at(tokens[pos].input, "数値でも識別子でもないトークンです");
}
