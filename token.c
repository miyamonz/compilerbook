#include "9cc.h"

Token *new_token(TokenKind kind, Token *cur, char *str) {
  Token *tok = calloc(1, sizeof(Token));
  tok->kind = kind;
  tok->str = str;
  cur->next = tok;
  return tok;
}

Map *keywords;
Token *tokenize(char *p) {
  keywords = new_map();
  map_put(keywords, "return", (void *) TK_RETURN);
  map_put(keywords, "if", (void *) TK_IF);
  map_put(keywords, "else", (void *) TK_ELSE);
  map_put(keywords, "while", (void *) TK_WHILE);
  map_put(keywords, "for", (void *) TK_FOR);
  map_put(keywords, "int", (void *) TK_INT);
  map_put(keywords, "sizeof", (void *) TK_SIZEOF);

  Token head;
  head.next = NULL;
  Token *cur = &head;

  while (*p) {
    // 空白文字をスキップ
    if (isspace(*p)) {
      p++;
      continue;
    }

    if (strncmp(p, "==", 2) == 0) {
      cur = new_token(TK_EQ, cur, p);
      cur->len = 2;
      p += 2;
      continue;
    }
    if (strncmp(p, "!=", 2) == 0) {
      cur = new_token(TK_NE, cur, p);
      cur->len = 2;
      p += 2;
      continue;
    }
    if (strncmp(p, "<=", 2) == 0) {
      cur = new_token(TK_LE, cur, p);
      cur->len = 2;
      p += 2;
      continue;
    }
    if (strncmp(p, ">=", 2) == 0) {
      cur = new_token(TK_GE, cur, p);
      cur->len = 2;
      p += 2;
      continue;
    }

    if(is_alpha(*p) || (*p == '_')) {
      int len = 1;
      while(is_alnum(p[len]))
        len++;
      char *name = strndup(p, (size_t)len);
      int kind = (intptr_t) map_get(keywords, name);
      if(!kind)
        kind = TK_IDENT;
      cur = new_token(kind, cur, p);
      cur->len = len;
      cur->name = name;
      p += len;
      continue;
    }


    if (strchr("+-*/()<>;={},&[]", *p)) {
      cur = new_token(*p, cur, p);
      cur->len = 1;
      p++;
      continue;
    }

    if (isdigit(*p)) {
      cur = new_token(TK_NUM, cur, p);
      cur->val = strtol(p, &p, 10);
      continue;
    }

    error_at(p, "トークナイズできません");
  }

  new_token(TK_EOF, cur, p);
  return head.next;
}
