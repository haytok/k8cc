#include "k8cc.h"

char *user_input; // コンパイルエラーを表示するための変数
Token *token;

void error(char *string, char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);

    int counts = string - user_input;
    fprintf(stderr, "%s\n", user_input);
    fprintf(stderr, "%*s", counts, " ");
    fprintf(stderr, "^ ");
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

bool startswith(char *p, char *q) {
    return memcmp(p, q, strlen(q)) == 0;
}

// stdbool.h が必要
bool at_eof() { return token->kind == TK_EOF; }

// 演算子に関する処理を実装
bool consume(char *op) {
    if (
        token->kind != TK_RESERVED ||
        memcmp(token->string, op, token->len) ||
        strlen(op) != token->len
    ) {
        return false;
    }
    token = token->next;
    return true;
}

void expect(char *op) {
    if (
        token->kind != TK_RESERVED ||
        memcmp(token->string, op, token->len) ||
        strlen(op) != token->len
    ) {
        error(token->string, "'%c'ではありません。\n", op);
    }
    token = token->next;
}

int expect_number() {
    if (token->kind != TK_NUM) {
        error(token->string, "数ではありません。");
    }
    int value = token->value;
    token = token->next;
    return value;
}

// token を作成するのは基本的にこの関数を通す
Token *new_token(TokenKind kind, char *string, Token *old_token, int len) {
    Token *new_tkn = calloc(1, sizeof(Token));
    new_tkn->kind = kind;
    new_tkn->string = string;
    new_tkn->len = len;
    old_token->next = new_tkn;
    return new_tkn;
}

Token *tokenize() {
    char *string = user_input;
    // 連結リストで作成していく Token の初期化
    Token head;
    head.next = NULL;
    Token *current_token = &head;

    while (*string) {
        if (isspace(*string)) {
            *string++;
            continue;
        }
        // == != <= >= に関する処理
        if (
            startswith(string, "==") ||
            startswith(string, "!=") ||
            startswith(string, "<=") ||
            startswith(string, ">=")
        ) {
            current_token = new_token(TK_RESERVED, string, current_token, 2);
            string += 2;
            continue;
        }
        if (strchr("+-*/()<>;", *string)) {
            current_token = new_token(TK_RESERVED, string++, current_token, 1);
            continue;
        }
        if ('a' <= *string && *string <= 'z') {
            current_token = new_token(TK_IDENT, string++, current_token, 1);
            continue;
        }
        if (isdigit(*string)) {
            current_token = new_token(TK_NUM, string, current_token, 0);
            char *tmp = string;
            long int value = strtol(string, &string, 10); // string のポインタが進む
            current_token->value = value;
            current_token->len = string - tmp;
            continue;
        }
        error(string, "トークナイズできません。");
    }
    current_token = new_token(TK_EOF, string, current_token, 0);
    return head.next;
}
