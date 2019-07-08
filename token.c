#include "9cc.h"

Map *keywords;
void tokenize() {
  char *p = user_input;
  int i = 0;
  keywords = new_map();
  map_put(keywords, "return", (void *) TK_RETURN);
  map_put(keywords, "if", (void *) TK_IF);
  map_put(keywords, "else", (void *) TK_ELSE);
  map_put(keywords, "while", (void *) TK_WHILE);
  map_put(keywords, "for", (void *) TK_FOR);
  map_put(keywords, "int", (void *) TK_INT);

  while (*p) {
    // 空白文字をスキップ
    if (isspace(*p)) {
      p++;
      continue;
    }

    if (strncmp(p, "==", 2) == 0) {
      tokens[i].kind = TK_EQ;
      tokens[i].input = p;
      i++;
      p += 2;
      continue;
    }
    if (strncmp(p, "!=", 2) == 0) {
      tokens[i].kind = TK_NE;
      tokens[i].input = p;
      i++;
      p += 2;
      continue;
    }
    if (strncmp(p, "<=", 2) == 0) {
      tokens[i].kind = TK_LE;
      tokens[i].input = p;
      i++;
      p += 2;
      continue;
    }
    if (strncmp(p, ">=", 2) == 0) {
      tokens[i].kind = TK_GE;
      tokens[i].input = p;
      i++;
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
      tokens[i].kind = kind;
      tokens[i].input = p;
      tokens[i].name = name;
      i++;
      p += len;
      continue;
    }


    if (strchr("+-*/()<>;={},&", *p)) {
      tokens[i].kind = *p;
      tokens[i].input = p;
      i++;
      p++;
      continue;
    }

    if (isdigit(*p)) {
      tokens[i].kind = TK_NUM;
      tokens[i].input = p;
      tokens[i].val = strtol(p, &p, 10);
      i++;
      continue;
    }

    error_at(p, "トークナイズできません");
  }

  tokens[i].kind = TK_EOF;
  tokens[i].input = p;

}
