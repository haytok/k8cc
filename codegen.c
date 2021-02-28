#include "k8cc.h"

// Node を元にアセンブリを生成する
void gen(Node *node) {
    switch (node->kind) {
        case NODE_NUM:
            printf("  push %d\n", node->value);
            return;
        case NODE_EXPR_STMT:
            gen(node->lhs);
            // 見本はこっちの実装になってる。
            printf("  add rsp, 8\n");
            // printf("  pop rax\n");
            return;
        case NODE_RETURN:
            gen(node->lhs);
            printf("  pop rax\n");
            printf("  ret\n");
            return;
    }

    gen(node->lhs);
    gen(node->rhs);

    printf("  pop rdi\n");
    printf("  pop rax\n");

    switch (node->kind) {
        case NODE_ADD:
            printf("  add rax, rdi\n");
            break;
        case NODE_SUB:
            printf("  sub rax, rdi\n");
            break;
        case NODE_MUL:
            printf("  imul rax, rdi\n");
            break;
        case NODE_DIV:
            printf("  cqo\n");
            printf("  idiv rdi\n");
            break;
        // 比較演算
        case NODE_EQ:
            printf("  cmp rax, rdi\n");
            printf("  sete al\n");
            printf("  movzb rax, al\n");
            break;
        case NODE_NE:
            printf("  cmp rax, rdi\n");
            printf("  setne al\n");
            printf("  movzb rax, al\n");
            break;
        case NODE_LT:
            printf("  cmp rax, rdi\n");
            printf("  setl al\n");
            printf("  movzb rax, al\n");
            break;
        case NODE_LE:
            printf("  cmp rax, rdi\n");
            printf("  setle al\n");
            printf("  movzb rax, al\n");
            break;
        case NODE_ASSIGN:
            break;
        case NODE_LVAR:
            break;
        default:
            break;
    }

    printf("  push rax\n");
}

void codegen(Node *node) {
    printf(".intel_syntax noprefix\n");
    printf(".global main\n");
    printf("main:\n");

    for (Node *n = node; n; n=n->next) {
        gen(n);
        // printf("  pop rax\n");
    }

    printf("  ret\n");
}
