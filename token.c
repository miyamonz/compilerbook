#include "9cc.h"

void tokenize() {
  char *p = user_input;
  int i = 0;
  while (*p) {
    // 空白文字をスキップ
    if (isspace(*p)) {
      p++;
      continue;
    }

    if (strncmp(p, "return", 6) == 0 && !is_alnum(p[6])) {
      tokens[i].ty = TK_RETURN;
      tokens[i].input = p;
      i++;
      p += 6;
      continue;
    }

    if (strncmp(p, "==", 2) == 0) {
      tokens[i].ty = TK_EQ;
      tokens[i].input = p;
      i++;
      p += 2;
      continue;
    }
    if (strncmp(p, "!=", 2) == 0) {
      tokens[i].ty = TK_NE;
      tokens[i].input = p;
      i++;
      p += 2;
      continue;
    }
    if (strncmp(p, "<=", 2) == 0) {
      tokens[i].ty = TK_LE;
      tokens[i].input = p;
      i++;
      p += 2;
      continue;
    }
    if (strncmp(p, ">=", 2) == 0) {
      tokens[i].ty = TK_GE;
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
      tokens[i].ty = TK_IDENT;
      tokens[i].input = p;
      tokens[i].name = name;
      i++;
      p += len;
      continue;
    }


    if (strchr("+-*/()<>;=", *p)) {
      tokens[i].ty = *p;
      tokens[i].input = p;
      i++;
      p++;
      continue;
    }

    if (isdigit(*p)) {
      tokens[i].ty = TK_NUM;
      tokens[i].input = p;
      tokens[i].val = strtol(p, &p, 10);
      i++;
      continue;
    }

    error_at(p, "トークナイズできません");
  }

  tokens[i].ty = TK_EOF;
  tokens[i].input = p;

}
