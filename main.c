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

  vars = new_map();
  bpoff = 0;
  label = 0;

  printf(".intel_syntax noprefix\n");
  printf(".global main\n");
  printf("main:\n");

  //プロローグ
  // 変数26個分の領域を確保する
  printf("  push rbp\n");
  printf("  mov rbp, rsp\n");
  printf("  sub rsp, 208\n");

   // 先頭の式から順にコード生成
  for (int i = 0; code[i]; i++) {
    gen(code[i]);

    // 式の評価結果としてスタックに一つの値が残っている
    // はずなので、スタックが溢れないようにポップしておく
    printf("  pop rax\n");
  }

  // エピローグ
  printf("  mov rsp, rbp\n");
  printf("  pop rbp\n");

  printf("  ret\n");
  return 0;
}
