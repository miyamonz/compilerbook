#include "9cc.h"

void gen_lval(Node *node) {
  if (node->ty != ND_IDENT)
    error("代入の左辺値が変数ではありません");

  if(!map_get(vars, node->name)) {
    bpoff += 8;
    map_put(vars, node->name, (void *)(intptr_t)bpoff);
  }
  int offset = (int)(intptr_t)map_get(vars,node->name);
  printf("  mov rax, rbp\n");
  printf("  sub rax, %d\n", offset);
  printf("  push rax\n");
}

void gen(Node *node) {
  if (node->ty == ND_NUM) {
    printf("  push %d\n", node->val);
    return;
  }

  if (node->ty == ND_IDENT) {
    gen_lval(node);
    printf("  pop rax\n");
    printf("  mov rax, [rax]\n");
    printf("  push rax\n");
    return;
  }

  if (node->ty == ND_CALL) {
    printf("  push rbx\n");
    printf("  push rbp\n");
    printf("  push rsp\n");
    printf("  push r12\n");
    printf("  push r13\n");
    printf("  push r14\n");
    printf("  push r15\n");

    char *arg[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};
    int len = 6;
    for (int i = 0; i < len; i++) {
      Node *expr = (Node *)node->args->data[i];
      if(!expr) break;
      gen(expr);
      printf("  pop %s\n", arg[i]);
    }
    printf("  mov rax, 0\n");
    printf("  call %s\n", node->name);
    printf("  mov r10, rax\n");

    printf("  pop r15\n");
    printf("  pop r14\n");
    printf("  pop r13\n");
    printf("  pop r12\n");
    printf("  pop rsp\n");
    printf("  pop rbp\n");
    printf("  pop rbx\n");

    printf("  push r10\n");
    return;
  }

  if (node->ty == ND_RETURN) {
    gen(node->lhs);
    printf("  pop rax\n");
    printf("  mov rsp, rbp\n");
    printf("  pop rbp\n");
    printf("  ret\n");
    return;
  }
  if (node->ty == ND_IF) {
    gen(node->cond);
    printf("  pop rax\n");
    printf("  cmp rax, 0\n");
    if(!node->els) {
      printf("  je .Lend%d\n", label);
      gen(node->then);
      printf(".Lend%d:\n", label);
    } else {
      printf("  je .Lelse%d\n", label);
      gen(node->then);
      printf("  jmp .Lend%d\n", label);
      printf(".Lelse%d:\n", label);
      gen(node->els);
      printf(".Lend%d:\n", label);
    }
    label++;
    return;
  }

  if (node->ty == ND_WHILE) {
    printf(".Lbegin%d:\n", label);
    gen(node->cond);
    printf("  pop rax\n");
    printf("  cmp rax, 0\n");
    printf("  je .Lend%d\n", label);
    gen(node->body);
    printf("  jmp .Lbegin%d\n", label);
    printf(".Lend%d:\n", label);
    label++;
    return;
  }

  if (node->ty == ND_FOR) {
    if(node->init)
      gen(node->init);
    printf(".Lbegin%d:\n", label);
    if(node->cond)
      gen(node->cond);
    printf("  pop rax\n");
    printf("  cmp rax, 0\n");
    printf("  je .Lend%d\n", label);
    gen(node->body);
    if(node->inc)
      gen(node->inc);
    printf("  jmp .Lbegin%d\n", label);
    printf(".Lend%d:\n", label);
    label++;
    return;
  }

  if (node->ty == ND_BLOCK) {

    int len = sizeof(node->stmts) / sizeof(node->stmts[0]);
    for(int i=0; i<len; i++) {
      if(!node->stmts[i]) break;
      gen(node->stmts[i]);
      printf("  pop rax\n");
    }
    return;
  }

  if (node->ty == '=') {
    gen_lval(node->lhs);
    gen(node->rhs);

    printf("  pop rdi\n");
    printf("  pop rax\n");
    printf("  mov [rax], rdi\n");
    printf("  push rdi\n");
    return;
  }

  gen(node->lhs);
  gen(node->rhs);

  printf("  pop rdi\n");
  printf("  pop rax\n");

  switch (node->ty) {
    case '+':
      printf("  add rax, rdi\n");
      break;
    case '-':
      printf("  sub rax, rdi\n");
      break;
    case '*':
      printf("  imul rdi\n");
      break;
    case '/':
      printf("  cqo\n");
      printf("  idiv rdi\n");
      break;
    case ND_EQ:
      printf("  cmp rax, rdi\n");
      printf("  sete al\n");
      printf("  movzb rax, al\n");
      break;
    case ND_NE:
      printf("  cmp rax, rdi\n");
      printf("  setne al\n");
      printf("  movzb rax, al\n");
      break;
    case ND_LE:
      printf("  cmp rax, rdi\n");
      printf("  setle al\n");
      printf("  movzb rax, al\n");
      break;
    case '<':
      printf("  cmp rax, rdi\n");
      printf("  setl al\n");
      printf("  movzb rax, al\n");
      break;
  }

  printf("  push rax\n");
}
