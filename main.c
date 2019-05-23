#include "9cc.h"

int main(int argc, char **argv) {
  if (argc == 2 && !strcmp(argv[1], "-test")) {
    runtest();
    return 0;
  }
  if (argc != 2) {
    error("引数の個数が正しくありません");
    return 1;
  }

  user_input = argv[1];
  tokenize();
  program();

  printf(".intel_syntax noprefix\n");
  printf(".global main\n");
  printf("main:\n");

   // 先頭の式から順にコード生成
  for (int i = 0; code[i]; i++) {
    gen(code[i]);

    // 式の評価結果としてスタックに一つの値が残っている
    // はずなので、スタックが溢れないようにポップしておく
    printf("  pop rax\n");
  }

  printf("  ret\n");
  return 0;
}
