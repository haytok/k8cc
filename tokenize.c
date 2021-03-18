#include "k8cc.h"

char *user_input; // コンパイルエラーを表示するための変数
Token *token;

void verror_at(char *string, char *fmt, va_list ap) {
    int counts = string - user_input;
    fprintf(stderr, "%s\n", user_input);
    fprintf(stderr, "%*s", counts, " ");
    fprintf(stderr, "^ ");
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

void error_at(char *string, char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    verror_at(string, fmt, ap);
}

void error_token(Token *token, char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    if (token) {
        verror_at(token->string, fmt, ap);
    }
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
Token *peek(char *op) {
    if (
        token->kind != TK_RESERVED ||
        memcmp(token->string, op, token->len) ||
        strlen(op) != token->len
    ) {
        return NULL;
    }
    return token;
}

// 引数の文字列が予約語であれば token を進める
// そうでなかったら NULL を返す
Token *consume(char *op) {
    if (!peek(op)) {
        return NULL;
    }
    Token *tkn = token;
    token = token->next;
    return tkn;
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

// 引数の文字列が予約語であれば token を進める
// そうでなかったらエラーを吐く
void expect(char *op) {
    if (!peek(op)) {
        error_token(token, "Invalid token in expect function due to '%c'.\n", op);
    }
    token = token->next;
}

int expect_number() {
    if (token->kind != TK_NUM) {
        error_token(token, "Invalid token in expect_number function.");
    }
    int value = token->value;
    token = token->next;
    return value;
}

char *expect_ident() {
    if (token->kind != TK_IDENT) {
        error_token(token, "Invalid token in expect_ident function.");
    }
    char *function_name = strndup(token->string, token->len);
    token = token->next;
    return function_name;
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
    static char *keyword[] = {
        "return",
        "if",
        "else",
        "while",
        "for",
        "int",
        "char",
        "sizeof"
    };
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
        if (strchr("+-*/()<>;={},&[]", *string)) {
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
        if (*string == '"') {
            char *start = string++;
            while (*string && *string != '"') {
                string++;
            }
            if (!*string) {
                error_at(string, "unclosed string literal");
            }
            string++;
            current_token = new_token(TK_STR, start, current_token, string - start);
            current_token->contents = strndup(start + 1, string - start - 2);
            current_token->cont_len = string - start - 1; // -1 にしているのはおそらく後で for 文で回す時の評価式で < を使用したいからと思われる。
            continue;
        }
        error_at(string, "トークナイズできません。");
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
