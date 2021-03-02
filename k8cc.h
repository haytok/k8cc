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
  TK_IDENT, // 変数
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
    NODE_RETURN, // return
    NODE_EXPR_STMT, // expression statement
    NODE_LVAR, // Local variable
    NODE_ASSIGN, // =
} NodeKind;

typedef struct Node Node;

struct Node {
    NodeKind kind;
    Node *lhs; // left-hand side
    Node *rhs; // right-hand side
    int value;
    Node *next;
    int offset;
    char name;
};

// Node に関する宣言
Node *new_node(NodeKind kind);
Node *new_node_binary(NodeKind kind, Node *lhs, Node *rhs);
Node *new_node_num(int value);

// 文法に関する宣言
Node *program();

// token を操作する関数
void error(char *string, char *fmt, ...); // 引数も同時に書かないとコンパイラが認識できない。
bool at_eof();
bool consume(char *op);
Token *consume_ident();
void expect(char *op);
int expect_number();
Token *tokenize();

// アセンブリを作成する関数
void codegen(Node *node);

// global 変数
extern char *user_input;
extern Token *token;
