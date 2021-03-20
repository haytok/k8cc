#include "k8cc.h"

char *user_input; // コンパイルエラーを表示するための変数
Token *token;
char *filename;

char *read_file(char *filename) {
    FILE *fd = fopen(filename, "r");
    if (!fd) {
        // error 関数の man とサンプルコードの引数が一致しないのが疑問。
        error("cannot open %s: %s", filename, strerror(errno));
    }
    int max_size = 10 * 1024 * 1024;
    char *buf = malloc(max_size);
    size_t n = fread(buf, 1, max_size - 2, fd);
    if (!feof(fd)) {
        error("%s: file too large", filename);
    }
    if (n == 0 || buf[n-1] != '\n') {
        buf[n++] = '\n';
    }
    buf[n] = '\0';
    return buf;
}

// この関数はよくわからん
// n + align - 1　を計算しているだけのような気がする
int align_to(int n, int align) {
  return (n + align - 1) & ~(align - 1);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "引数の個数が正しくありません。\n");
        return 1;
    }

    filename = argv[1];
    user_input = read_file(filename);
    token = tokenize();
    Program *prog = program();
    add_type(prog);

    for (Function *f = prog->functions; f; f = f->next) {
        int offset = 0;
        for (VarList *v = f->locals; v; v = v->next) {
            offset += size_of(v->var->ty);
            v->var->offset = offset;
        }
        f->stack_size = align_to(offset, 8); // この処理はよくわからん
    }

    codegen(prog);

    return 0;
}
