#include "k8cc.h"

int label_seq = 0;

// 左辺値のアセンブリを出力
void gen_lval(Node *node) {
    if (node->kind != NODE_VAR) {
        error("Invalid lvalue.", "error");
    }
    // node->var->name と node->var->offset に変数に関する必要なパラメータが入っている。
    printf("  mov rax, rbp\n");
    // printf("  lea rax [rbp-%d]\n", offset); // このアセンブリでも可
    printf("  sub rax, %d\n", node->var->offset);
    printf("  push rax\n"); // 最終的にスタックのトップには変数のアドレスが積まれている。
    return;
}

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
            // printf("  pop rax\n"); // この実装でもテストは通る。
            return;
        case NODE_ASSIGN:
            gen_lval(node->lhs);
            gen(node->rhs);

            printf("  pop rdi\n");
            printf("  pop rax\n");
            printf("  mov [rax], rdi\n");
            printf("  push rdi\n");
            return;
        // a + z の処理の時に呼ばれる。
        case NODE_VAR:
            gen_lval(node);

            printf("  pop rax\n");
            printf("  mov rax, [rax]\n");
            printf("  push rax\n");
            return;
        case NODE_IF: {
            int seq = label_seq++;
            if (node->els) {
                // 条件式の処理
                gen(node->condition);
                printf("  pop rax\n");
                printf("  cmp rax, 0\n");
                printf("  je .Lelse%d\n", seq);
                gen(node->then);
                printf("  jmp .Lend%d\n", seq);
                printf("  .Lelse%d:\n", seq);
                gen(node->els);
                printf("  .Lend%d:\n", seq);
                return;
            } else {
                // 条件式の処理
                gen(node->condition);
                printf("  pop rax\n");
                printf("  cmp rax, 0\n");
                printf("  je .Lend%d\n", seq);
                // B の処理
                gen(node->then);
                printf("  .Lend%d:\n", seq);
                return;
            }
        }
        case NODE_WHILE: {
            int seq = label_seq++;
            printf("  .Lbegin%d:\n", seq);
            gen(node->condition);
            printf("  pop rax\n");
            printf("  cmp rax, 0\n");
            printf("  je .Lend%d\n", seq);
            gen(node->then);
            printf("  jmp .Lbegin%d\n", seq);
            printf("  .Lend%d:\n", seq);
            return;
        }
        case NODE_FOR: {
            int seq = label_seq++;
            if (node->init) {
                gen(node->init);
            }
            printf("  .Lbegin%d:\n", seq);
            if (node->condition) {
                gen(node->condition);
                printf("  pop rax\n");
                printf("  cmp rax, 0\n");
                printf("  je .Lend%d\n", seq);
            }
            gen(node->then);
            if (node->inc) {
                gen(node->inc);
            }
            printf("  jmp .Lbegin%d\n", seq);
            printf("  .Lend%d:\n", seq); // アセンブラ内の : は必須
            return;
        }
        case NODE_BLOCK: {
            for (Node *n = node->body; n; n=n->next) {
                gen(n);
            }
            return;
        }
        case NODE_RETURN:
            gen(node->lhs);

            printf("  pop rax\n");
            // printf("  ret\n");
            printf("  jmp .Lreturn\n");
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
        default:
            break;
    }

    printf("  push rax\n");
}

void codegen(Program *prog) {
    printf(".intel_syntax noprefix\n");
    printf(".global main\n");
    printf("main:\n");

    // プロローグ
    printf("  push rbp\n");
    printf("  mov rbp, rsp\n");
    printf("  sub rsp, %d\n", prog->stack_size);


    for (Node *n = prog->node; n; n=n->next) {
        gen(n);
        // printf("  pop rax\n");
    }

    // エピローグ
    printf(".Lreturn:\n");
    printf("  mov rsp, rbp\n");
    printf("  pop rbp\n");

    printf("  ret\n");
}
