#include "9cc.h"
#include "assert.h"

static Type int_ty = {INT, NULL};

typedef struct {
  Type *ty;
  int offset;
} Var;

int isPtr(Node *node) {
  if (node->op != ND_IDENT) return 0;
  Var *var = (Var *)map_get(vars,node->name);
  return var->ty->ty == PTR;
}
int size_of(Type *ty) {
  if (ty->ty == INT)
    return 4;
  assert(ty->ty == PTR);
  return 8;
}

char *arg[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};
int len = sizeof(arg)/sizeof(*arg);

Var *gen_vardef(Node *node) {
  if (node->op != ND_VARDEF)
    error("変数宣言ではありません");

  if(map_get(vars, node->name))
    error("%s is already defined.", node->name);

  bpoff += 8;
  Var *var = malloc(sizeof(Var));
  var->ty = node->ty;
  var->offset = bpoff;
  map_put(vars, node->name, (void *)var);
  return var;
}

//gen_lval pushes pointer to variable
void gen_lval(Node *node) {
  if (node->op == ND_IDENT) {
    if(!map_get(vars, node->name))
      error("variable %s is not defined yet.", node->name);

    Var *var = (Var *)map_get(vars,node->name);
    printf("  mov rax, rbp\n");
    printf("  sub rax, %d\n", var->offset);
    printf("  push rax\n");
    return;
  }
  if(node->op == ND_DEREF) {
    gen(node->lhs);
    return;
  }

  error("代入の左辺値が不正です");
}

void gen(Node *node) {
  if (node->op == ND_NUM) {
    printf("  push %d\n", node->val);
    return;
  }

  if (node->op == ND_IDENT) {
    gen_lval(node);
    printf("  pop rax\n");
    printf("  mov rax, [rax]\n");
    printf("  push rax\n");
    return;
  }

  if (node->op == ND_VARDEF) {
    gen_vardef(node);
    return;
  }

  if (node->op == ND_CALL) {
    printf("  push rbx\n");
    printf("  push rbp\n");
    printf("  push rsp\n");
    printf("  push r12\n");
    printf("  push r13\n");
    printf("  push r14\n");
    printf("  push r15\n");

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

  if (node->op == ND_RETURN) {
    gen(node->lhs);
    printf("  pop rax\n");
    printf("  mov rsp, rbp\n");
    printf("  pop rbp\n");
    printf("  ret\n");
    return;
  }
  if (node->op == ND_IF) {
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

  if (node->op == ND_WHILE) {
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

  if (node->op == ND_FOR) {
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

  if (node->op == ND_BLOCK) {

    int len = sizeof(node->stmts) / sizeof(node->stmts[0]);
    for(int i=0; i<len; i++) {
      if(!node->stmts[i]) break;
      gen(node->stmts[i]);
      printf("  pop rax\n");
    }
    return;
  }

  if (node->op == '=') {
    gen_lval(node->lhs);
    gen(node->rhs);

    printf("  pop rdi\n");
    printf("  pop rax\n");
    printf("  mov [rax], rdi\n");
    printf("  push rdi\n");
    return;
  }
  if (node->op == ND_DEREF) {
    gen(node->lhs);
    printf("  pop rax\n");
    printf("  mov rax, [rax]\n");
    printf("  push rax\n");
    return;
  }
  if (node->op == ND_ADDR) {
    Var *var = (Var *)map_get(vars,node->lhs->name);
    printf("  mov rax, rbp\n");
    printf("  sub rax, %d\n", var->offset);
    printf("  push rax\n");
    return;
  }

  gen(node->lhs);
  gen(node->rhs);

  printf("  pop rdi\n");
  printf("  pop rax\n");

  Type *ltype = node->lhs->ty;
  Type *rtype = node->rhs->ty;
  int lIsPtr = isPtr(node->lhs);
  int rIsPtr = isPtr(node->rhs);

  node->ty = &int_ty;
  switch (node->op) {
    case '+':
      if(lIsPtr && rIsPtr)
        error(" pointer + pointer is invalid");
    case '-':
      if(lIsPtr && !rIsPtr) {
        printf("  push rax\n"); // push lhs

        //rhs * size
        printf("  mov rax, rdi\n");
        printf("  push %d\n", size_of(rtype));
        printf("  pop rdi\n");
        printf("  imul rdi\n");
        printf("  mov rdi, rax\n");
        node->ty = rtype;

        printf("  pop rax\n"); // pop lhs
      }
      if(!lIsPtr && rIsPtr) {
        printf("  push rdi\n"); // push rhs

        //lhs * size
        printf("  push %d\n", size_of(ltype));
        printf("  pop rdi\n");
        printf("  imul rdi\n");

        printf("  pop rdi\n"); // pop rhs
        node->ty = ltype;
      }

      printf("  %s rax, rdi\n", node->op == '+' ? "add" : "sub");
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

void gen_func(Node *node) {
  if (node->op != ND_FUNC)
    error("関数ではありません");

  printf("%s:\n", node->name);

  //プロローグ
  printf("  push rbp\n");
  printf("  mov rbp, rsp\n");
  printf("  sub rsp, 208\n");

  // 変数代入と同様のコードを作り、値はABIに基づいて代入する
  for(int i=0; i<len; i++) {
    Node *param = (Node *)node->args->data[i];
    if(!param) break;

    Var *var = gen_vardef(param);
    printf("  mov rax, rbp\n");
    printf("  sub rax, %d\n", var->offset);
    printf("  push rax\n");

    printf("  pop rax\n");
    printf("  mov [rax], %s\n", arg[i]);
  }

  int i=0;
  while(node->body->stmts[i]) {
    gen(node->body->stmts[i]);
    i++;
    // 式の評価結果がスタックに一つの値が残っている
    // 溢れないようにポップしておく
    printf("  pop rax\n");
  }
  // エピローグ
  printf("  mov rsp, rbp\n");
  printf("  pop rbp\n");

  printf("  ret\n");
}
