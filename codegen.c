#include "k8cc.h"

void gen(Node *node);

int label_seq = 0;
char *arg_register[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};
char *function_name;

// 左辺値のアセンブリを出力
void gen_lval(Node *node) {
    switch (node->kind) {
        case NODE_VAR: {
            // node->var->name と node->var->offset に変数に関する必要なパラメータが入っている。
            printf("  mov rax, rbp\n");
            // printf("  lea rax [rbp-%d]\n", offset); // このアセンブリでも可
            printf("  sub rax, %d\n", node->var->offset);
            printf("  push rax\n"); // 最終的にスタックのトップには変数のアドレスが積まれている。
            return;
        }
        // gen 関数の NODE_ASSIGN の gen_lval で呼び出される。
        case NODE_DEREF: {
            gen(node->lhs);
            return;
        }
    }
}

// Node を元にアセンブリを生成する
void gen(Node *node) {
    switch (node->kind) {
        case NODE_NULL:
            return;
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
        case NODE_FUNCALL: {
            int arg_n = 0;
            for (Node *arg = node->args; arg; arg = arg->next) {
                // printf("  push %d\n", arg->value); // return add(3 + 5, 5); のケースで落ちるので以下の処理のほうが適切。
                gen(arg);
                arg_n++;
            }
            for (int i = arg_n - 1; i >= 0; i--) {
                printf("  pop %s\n", arg_register[i]);
            }
            int seq = label_seq++;
            printf("  mov rax, rsp\n");
            printf("  and rax, 15\n");
            printf("  jnz .Lcall%d\n", seq);
            printf("  mov rax, 0\n");
            printf("  call %s\n", node->function_name);
            printf("  jmp .Lend%d\n", seq);
            // 16 バイトになっていない時の処理
            printf("  .Lcall%d:\n", seq);
            printf("  sub rsp, 8\n");
            printf("  mov rax, 0\n");
            printf("  call %s\n", node->function_name);
            printf("  add rsp, 8\n");
            printf("  .Lend%d:\n", seq);
            printf("  push rax\n"); // 関数で計算した結果をスタックに積む解釈で大丈夫か？
            return;
        }
        // &
        case NODE_ADDRESS: {
            gen_lval(node->lhs); // 次は変数名のケースの処理
            return;
        }
        // * (変数や () が来る)
        case NODE_DEREF: {
            gen(node->lhs);
            printf("  pop rax\n");
            printf("  mov rax, [rax]\n");
            printf("  push rax\n");
            return;
        }
        case NODE_RETURN:
            gen(node->lhs);

            printf("  pop rax\n");
            // printf("  ret\n");
            printf("  jmp .Lreturn.%s\n", function_name);
            return;
    }

    gen(node->lhs);
    gen(node->rhs);

    printf("  pop rdi\n");
    printf("  pop rax\n");

    switch (node->kind) {
        case NODE_ADD:
            if (node->type->kind == TYPE_PTR) {
                printf("  imul rdi, 8\n");
            }
            printf("  add rax, rdi\n");
            break;
        case NODE_SUB:
            if (node->type->kind == TYPE_PTR) {
                printf("  imul rdi, 8\n");
            }
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

void codegen(Function *prog) {
    printf(".intel_syntax noprefix\n");

    for (Function *f = prog; f; f = f->next) {
        function_name = f->function_name;
        printf(".global %s\n", function_name);
        printf("%s:\n", function_name);

        // プロローグ
        printf("  push rbp\n");
        printf("  mov rbp, rsp\n");
        printf("  sub rsp, %d\n", f->stack_size);

        // 呼び出された側での引数の値をローカル変数に存在するものとしてコンパイル
        // 引数ありの関数呼び出しを行った際に走る処理
        int i = 0;
        for (VarList *p = f->params; p; p = p->next) {
            Var *v = p->var;
            // v->name には x, y などの定義された関数の引数名が入っている。
            printf("  mov [rbp-%d], %s\n", v->offset, arg_register[i++]);
        }

        for (Node *n = f->node; n; n = n->next) {
            gen(n);
            // printf("  pop rax\n");
        }

        // エピローグ
        printf(".Lreturn.%s:\n", function_name);
        printf("  mov rsp, rbp\n");
        printf("  pop rbp\n");

        printf("  ret\n");
    }
}
