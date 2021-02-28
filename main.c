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
    Node *node = program();
    codegen(node);

    return 0;
}
