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
    Program *prog = program();
    add_type(prog);

    for (Function *f = prog->functions; f; f = f->next) {
        int offset = 0;
        for (VarList *v = f->locals; v; v = v->next) {
            offset += size_of(v->var->ty);
            v->var->offset = offset;
        }
        f->stack_size = offset;
    }

    codegen(prog);

    return 0;
}
