#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>

typedef enum {
  TK_RESERVED,
  TK_NUM,
  TK_EOF,
} TokenKind;

typedef struct Token Token;

struct Token {
    TokenKind kind;
    char *string;
    int value;
    Token *next;
};

Token *token;
char *user_input;

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

Token *new_token(TokenKind kind, char *string, Token *old_token) {
    Token *new_tkn = calloc(1, sizeof(Token));
    new_tkn->kind = kind;
    new_tkn->string = string;
    old_token->next = new_tkn;
    return new_tkn;
}

Token *tokenize(char *string) {
    // 連結リストで作成していく Token の初期化
    Token head;
    head.next = NULL;
    Token *current_token = &head;

    while (*string) {
        if (isspace(*string)) {
            *string++;
            continue;
        }
        if (*string == '+' || *string == '-') {
            current_token = new_token(TK_RESERVED, string++, current_token);
            continue;
        }
        if (isdigit(*string)) {
            current_token = new_token(TK_NUM, string, current_token);
            long int value = strtol(string, &string, 10);
            current_token->value = value;
            continue;
        }
        error(token->string, "トークナイズできません。");
    }
    current_token = new_token(TK_EOF, string, current_token);
    return head.next;
}

int expect_number() {
    if (token->kind != TK_NUM) {
        error(token->string, "数ではありません。");
    }
    int value = token->value;
    token = token->next;
    return value;
}

// stdbool.h が必要
bool at_eof() { return token->kind == TK_EOF; }

bool consume(char op) {
    if (token->kind != TK_RESERVED || token->string[0] != op) {
        return false;
    }
    token = token->next;
    return true;
}

void expect(char op) {
    if (token->kind != TK_RESERVED || token->string[0] != op) {
        error(token->string, "'%c'ではありません。\n", op);
    }
    token = token->next;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "引数の個数が正しくありません。\n");
        return 1;
    }

    printf(".intel_syntax noprefix\n");
    printf(".global main\n");
    printf("main:\n");

    token = tokenize(argv[1]);
    user_input = argv[1];

    printf("  mov rax, %d\n", expect_number());

    while(!at_eof()) {
        if (consume('+')) {
            printf("  add rax, %d\n", expect_number());
            continue;
        }
        expect('-');
        printf("  sub rax, %d\n", expect_number());
    }

    printf("  ret\n");
    return 0;
}
