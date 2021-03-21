#include "k8cc.h"

void gen(Node *node);

int label_seq = 0;
char *arg_register1[] = {"dil", "sil", "dl", "cl", "r8b", "r9b"};
char *arg_register8[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};
char *function_name;

void gen_addr(Node *node) {
    switch (node->kind) {
        case NODE_VAR: {
            Var *var = node->var;
            if (var->is_local) {
                // node->var->name と node->var->offset に変数に関する必要なパラメータが入っている。
                printf("  mov rax, rbp\n");
                // printf("  lea rax [rbp-%d]\n", offset); // このアセンブリでも可
                printf("  sub rax, %d\n", var->offset);
                printf("  push rax\n"); // 最終的にスタックのトップには変数のアドレスが積まれている。
            } else {
                printf("  push offset %s\n", var->name);
            }
            return;
        }
        // gen 関数の NODE_ASSIGN の gen_lval で呼び出される。
        case NODE_DEREF: {
            gen(node->lhs);
            return;
        }
        case NODE_MEMBER: {
            gen_addr(node->lhs);
            printf("  pop rax\n");
            printf("  add rax, %d\n", node->member->offset);
            printf("  push rax\n");
            return;
        }
    }
}

// 左辺値のアセンブリを出力
void gen_lval(Node *node) {
    if (node->type->kind == TYPE_ARRAY) {
        error_token(node->token, "not an lvalue");
    }
    gen_addr(node);
}

void load(Type *ty) {
    printf("  pop rax\n");

    if (size_of(ty) == 1) {
        printf("  movsx eax, byte ptr [rax]\n");
    } else if (size_of(ty) == 8) {
        printf("  mov rax, [rax]\n");
    }

    printf("  push rax\n");
}

void store(Type *ty) {
    printf("  pop rdi\n");
    printf("  pop rax\n");

    if (size_of(ty) == 1) {
        printf("  mov [rax], dil\n");
    } else if (size_of(ty) == 8) {
        printf("  mov [rax], rdi\n");
    }

    printf("  push rdi\n");
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

            store(node->type);
            return;
        // a + z の処理の時に呼ばれる。
        case NODE_MEMBER:
        case NODE_VAR:
            gen_addr(node);
            // 現時点では Array に代入は実装しない。
            if (node->type->kind != TYPE_ARRAY) {
                load(node->type);
            }
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
        case NODE_BLOCK:
        case NODE_STMT_EXPR:
            for (Node *n = node->body; n; n=n->next) {
                gen(n);
            }
            return;
        case NODE_FUNCALL: {
            int arg_n = 0;
            for (Node *arg = node->args; arg; arg = arg->next) {
                // printf("  push %d\n", arg->value); // return add(3 + 5, 5); のケースで落ちるので以下の処理のほうが適切。
                gen(arg);
                arg_n++;
            }
            for (int i = arg_n - 1; i >= 0; i--) {
                printf("  pop %s\n", arg_register8[i]);
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
            gen_addr(node->lhs); // 次は変数名のケースの処理
            return;
        }
        // * (変数や () が来る)
        case NODE_DEREF: {
            gen(node->lhs);
            // 現時点では Array に代入は実装しない。
            if (node->type->kind != TYPE_ARRAY) {
                load(node->type);
            }
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
            if (node->type->base) {
                printf("  imul rdi, %d\n", size_of(node->type->base));
            }
            printf("  add rax, rdi\n");
            break;
        case NODE_SUB:
            if (node->type->base) {
                printf("  imul rdi, %d\n", size_of(node->type->base));
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

// global 変数のアセンブリを吐き出す関数
// ex)
// x:
//   .zero 4
void emit_data(Program *prog) {
    printf(".data\n");

    // グローバル変数のアセンブリを出力
    for (VarList *vl = prog->globals; vl; vl = vl->next) {
        Var *var = vl->var;
        printf("%s:\n", var->name);
        // contents が存在する時は文字列リテラルのケース
        if (!var->contents) {
            printf("  .zero %d\n", size_of(var->ty));
            continue;
        }
        for (int i = 0; i < var->cont_len; i++) {
            printf("  .byte %d\n", var->contents[i]);
        }
    }
}

void load_arg(Var *var, int i) {
    int sz = size_of(var->ty);
    if (sz == 1) {
        printf("  mov [rbp-%d], %s\n", var->offset, arg_register1[i]);
    } else if (sz == 8) {
        printf("  mov [rbp-%d], %s\n", var->offset, arg_register8[i]);
    }
}

// 関数のアセンブリを吐き出す関数
void emit_text(Program *prog) {
    printf(".text\n");
    for (Function *f = prog->functions; f; f = f->next) {
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
            load_arg(v, i++);
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

void codegen(Program *prog) {
    printf(".intel_syntax noprefix\n");
    emit_data(prog);
    emit_text(prog);
}
