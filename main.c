#include "k8cc.h"

char *user_input; // コンパイルエラーを表示するための変数
Token *token;

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "引数の個数が正しくありません。\n");
        return 1;
    }

    user_input = argv[1];
    token = tokenize();
    Function *prog = program();

    for (Function *f = prog; f; f = f->next) {
        int offset = 0;
        for (Var *v = prog->var; v; v = v->next) {
            offset += 8;
            v->offset = offset;
        }
        prog->stack_size = offset;

    }

    codegen(prog);

    return 0;
}
