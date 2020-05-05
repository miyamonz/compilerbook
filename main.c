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
  token = tokenize(user_input);
  program();

  vars = new_map();
  bpoff = 0;
  label = 0;

  printf(".intel_syntax noprefix\n");
  printf(".global main\n");

  int i=0;
  while(funcs[i]) {
    gen_func(funcs[i]);
    i++;
  }

  return 0;
}
