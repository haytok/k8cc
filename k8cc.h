#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>

typedef struct Token Token;
typedef struct Node Node;
typedef struct Var Var;
typedef struct VarList VarList;
typedef struct Function Function;
typedef struct Type Type;
typedef struct Program Program;

typedef enum {
  TK_RESERVED,
  TK_NUM,
  TK_STR,
  TK_EOF,
  TK_IDENT, // 変数
} TokenKind;

struct Token {
    TokenKind kind;
    char *string;
    int value;
    Token *next;
    int len;
    // char で使用する変数
    char *contents;
    int cont_len;
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
    NODE_FUNCALL, // function
    NODE_DEREF, // *
    NODE_ADDRESS, // &
    NODE_NULL, // 変数宣言で使用
    NODE_SIZEOF,
    NODE_STMT_EXPR,
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
    Node *body; // block or statement expression
    char *function_name;
    Node *args; // args of function
    Token *token;
    Type *type;
};

struct Var {
    // Var *next;
    char *name;
    Type *ty;
    bool is_local;
    int offset;
    // char で使用する変数
    char *contents;
    int cont_len;
};

struct VarList {
    VarList *next;
    Var *var;
};

struct Function {
    Function *next;
    char *function_name;
    Node *node;
    VarList *locals;
    int stack_size;
    VarList *params;
};

typedef enum {
    TYPE_INT,
    TYPE_CHAR,
    TYPE_PTR,
    TYPE_ARRAY,
} TypeKind;

struct Type {
    TypeKind kind;
    Type *base;
    size_t array_size;
};

struct Program {
    Function *functions;
    VarList *globals;
};

// 文法に関する宣言
Program *program();

// token を操作する関数
void error_at(char *string, char *fmt, ...); // 引数も同時に書かないとコンパイラが認識できない。
void error_token(Token *tkn, char *fmt, ...);
bool at_eof();
Token *consume(char *op);
Token *consume_ident();
void expect(char *op);
int expect_number();
char *expect_ident();
Token *tokenize();
char *strndup(char *string, int len);
Token *peek(char *op);

// アセンブリを作成する関数
void codegen(Program *prog);

// global 変数
extern char *user_input;
extern Token *token;

// type を追加
Type *int_type();
Type *char_type();
Type *pointer_to(Type *base);
Type *array_of(Type *base, int array_size);
int size_of(Type *ty);
void add_type(Program *prog);
