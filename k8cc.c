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

typedef enum {
    NODE_ADD,
    NODE_SUB,
    NODE_MUL,
    NODE_DIV,
    NODE_NUM,
} NodeKind;

typedef struct Node Node;

struct Node {
    NodeKind kind;
    Node *lhs; // left-hand side
    Node *rhs; // right-hand side
    int value;
};

// 関数の循環参照のためのプロトタイプ宣言
Node *new_node(NodeKind kind, Node *lhs, Node *rhs);
Node *new_node_num(int value);
Node *expr();
Node *mul();
Node *unary();
Node *primary();

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
        if (strchr("+-*/()", *string)) {
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

// 構文解析
Node *new_node(NodeKind kind, Node *lhs, Node *rhs) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = kind;
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

Node *new_node_num(int value) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = NODE_NUM;
    node->value = value;
    return node;
}

Node *expr() {
    Node *node = mul();

    for (;;) {
        if (consume('+')) {
            node = new_node(NODE_ADD, node, mul());
        } else if (consume('-')) {
            node = new_node(NODE_SUB, node, mul());
        } else {
            return node; // 最終的に生成される node
        }
    }
}

Node *mul() {
    Node *node = unary();

    for (;;) {
        if (consume('*')) {
            node = new_node(NODE_MUL, node, unary());
        } else if (consume('/')) {
            node = new_node(NODE_DIV, node, unary());
        } else {
            return node; // 最終的に生成される node
        }
    }
}

// unary = ("+" | "-")? unary | primary
Node *unary() {
    if (consume('+')) {
        return unary();
    }
    if (consume('-')) {
        return new_node(NODE_SUB, new_node_num(0), unary());
    }
    return primary();
}

Node *primary() {
    if (consume('(')) {
        Node *node = expr();
        expect(')');
        return node; // この時点で token は ) の次の token を指している
    }
    return new_node_num(expect_number()); // この時点で token は数字の次の token を指している
}

// Node を元にアセンブリを生成する
void gen(Node *node) {
    if (node->kind == NODE_NUM) {
        printf("  push %d\n", node->value);
        return;
    }

    gen(node->lhs);
    gen(node->rhs);

    printf("  pop rdi\n");
    printf("  pop rax\n");

    switch (node->kind) {
        case NODE_ADD:
            printf("  add rax, rdi\n");
            break;
        case NODE_SUB:
            printf("  sub rax, rdi\n");
            break;
        case NODE_MUL:
            printf("  imul rax, rdi\n");
            break;
        case NODE_DIV:
            printf("  cqo\n");
            printf("  idiv rdi\n");
            break;
    }

    printf("  push rax\n");
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "引数の個数が正しくありません。\n");
        return 1;
    }

    printf(".intel_syntax noprefix\n");
    printf(".global main\n");
    printf("main:\n");

    user_input = argv[1]; // コンパイルエラーを表示するための変数
    token = tokenize(user_input);
    Node *node = expr();

    gen(node);

    printf("  pop rax\n");
    printf("  ret\n");
    return 0;
}
