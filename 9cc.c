#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

enum {
  TK_NUM = 256,
  TK_EOF,
};

typedef struct {
  int ty;
  int val;
  char *input;
} Token;

char *user_input;
Token tokens[100];

void tokenize() {
  char *p = user_input;
  int i = 0;
  while (*p) {
    // 空白文字をスキップ
    if (isspace(*p)) {
      p++;
      continue;
    }

    if (*p == '+' || *p == '-') {
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

    fprintf(stderr, "トークナイズできません\n");
    exit(1);
  }

  tokens[i].ty = TK_EOF;
  tokens[i].input = p;

}

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "引数の個数が正しくありません\n");
    return 1;
  }

  user_input = argv[1];
  tokenize();


  printf(".intel_syntax noprefix\n");
  printf(".global main\n");
  printf("main:\n");

  if (tokens[0].ty != TK_NUM)
    fprintf(stderr, "数ではありません\n");
  printf("  mov rax, %d\n", tokens[0].val);

  int i = 1;
  while(tokens[i].ty != TK_EOF) {
    if(tokens[i].ty == '+') {
      i++;
      if (tokens[i].ty != TK_NUM) {
        fprintf(stderr, "%c 数ではありません\n", *tokens[i].input);
        exit(1);
      }

      printf("  add rax, %d\n", tokens[i].val);
      i++;
      continue;
    }

    if(tokens[i].ty == '-') {
      i++;
      if (tokens[i].ty != TK_NUM) {
        fprintf(stderr, "%c 数ではありません\n", *tokens[i].input);
        exit(1);
      }
      printf("  sub rax, %d\n", tokens[i].val);
      i++;
      continue;
    }

    fprintf(stderr, "予期しない文字です: '%c'\n", *tokens[i].input);
    exit(1);
  }

  printf("  ret\n");
  return 0;
}
