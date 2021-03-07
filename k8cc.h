#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>

typedef struct Token Token;
typedef struct Node Node;
typedef struct Var Var;
typedef struct Program Program;

typedef enum {
  TK_RESERVED,
  TK_NUM,
  TK_EOF,
  TK_IDENT, // 変数
} TokenKind;

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
    NODE_VAR, // Local variable
    NODE_ASSIGN, // =
    NODE_IF, // if
    NODE_WHILE, // while
    NODE_FOR, // for
    NODE_BLOCK,
} NodeKind;

struct Node {
    NodeKind kind;
    Node *lhs; // left-hand side
    Node *rhs; // right-hand side
    int value;
    Node *next;
    int offset;
    Var *var;
    // if statement or while or for statement に関するメンバ
    Node *condition;
    Node *then;
    Node *els;
    Node *init;
    Node *inc;
    Node *body; // block
};

struct Var {
    Var *next;
    char *name;
    int offset;
};

struct Program {
    Node *node;
    Var *var;
    int stack_size;
};

// Node に関する宣言
Node *new_node(NodeKind kind);
Node *new_node_binary(NodeKind kind, Node *lhs, Node *rhs);
Node *new_node_num(int value);

// 文法に関する宣言
Program *program();

// token を操作する関数
void error(char *string, char *fmt, ...); // 引数も同時に書かないとコンパイラが認識できない。
bool at_eof();
bool consume(char *op);
Token *consume_ident();
void expect(char *op);
int expect_number();
Token *tokenize();
char *strndup(char *string, int len);

// アセンブリを作成する関数
void codegen(Program *prog);

// global 変数
extern char *user_input;
extern Token *token;
