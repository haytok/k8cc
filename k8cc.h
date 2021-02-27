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
    int len;
};

typedef enum {
    NODE_ADD, // +
    NODE_SUB, // -
    NODE_MUL, // *
    NODE_DIV, // /
    NODE_EQ, // ==
    NODE_NE, // not =
    NODE_LT, // <
    NODE_LE, // <=
    NODE_NUM, // Integer
} NodeKind;

typedef struct Node Node;

struct Node {
    NodeKind kind;
    Node *lhs; // left-hand side
    Node *rhs; // right-hand side
    int value;
};

// Node に関する宣言
Node *new_node(NodeKind kind);
Node *new_node_binary(NodeKind kind, Node *lhs, Node *rhs);
Node *new_node_num(int value);

// 文法に関する宣言
Node *expr();

// token を操作する関数
bool consume(char *op);
void expect(char *op);
int expect_number();
Token *tokenize();

// アセンブリを作成する関数
void codegen(Node *node);

// global 変数
extern char *user_input;
extern Token *token;
