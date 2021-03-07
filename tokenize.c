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

bool is_alpha(char c) {
    return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') ||  c == '_';
}

bool is_alnum(char c) {
    return is_alpha(c) || ('0' <= c && c <= '9');
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

// 一文字の変数のトークンに関する処理
Token *consume_ident() {
    if (token->kind != TK_IDENT) {
        return NULL;
    }
    Token *tok = token;
    token = token->next;
    return tok;
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

// キーワードを返す設計にする
char *starts_with_reserved(char *string) {
    static char *keyword[] = {"return", "if", "else", "while", "for"};
    int keyword_size = sizeof(keyword) / sizeof(*keyword);
    for (int i = 0; i < keyword_size; i++) {
        int len = strlen(keyword[i]);
        if (startswith(string, keyword[i]) && !is_alnum(string[len])) {
            return keyword[i];
        }
    }
    static char *opts[] = {"==", "!=", "<=", ">="};
    int opts_size = sizeof(opts) / sizeof(*opts);
    for (int i = 0; i < opts_size; i++) {
        if (startswith(string, opts[i])) {
            return opts[i];
        }
    }
    return NULL;
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
        // 予約語に対するトークナイズ
        char *reserved_keyword = starts_with_reserved(string);
        if (reserved_keyword) {
            int len = strlen(reserved_keyword);
            current_token = new_token(TK_RESERVED, string, current_token, len);
            string += len;
            continue;
        }
        if (strchr("+-*/()<>;={}", *string)) {
            current_token = new_token(TK_RESERVED, string++, current_token, 1);
            continue;
        }
        if (is_alpha(*string)) {
            char *start = string;
            string++;
            while(is_alnum(*string)) {
                string++;
            }
            current_token = new_token(TK_IDENT, start, current_token, string - start);
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

char *strndup(char *string, int len) {
    char *buf = malloc(len + 1);
    strncpy(buf, string, len);
    buf[len] = '\0';
    return buf;
}
