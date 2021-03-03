#include "k8cc.h"

Var *locals;

// 関数の循環参照のためのプロトタイプ宣言
Program *program();
Node *stmt();
Node *expr();
Node *assign();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *unary();
Node *primary();

// 構文解析
Node *new_node(NodeKind kind) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = kind;
    return node;
}

Node *new_node_binary(NodeKind kind, Node *lhs, Node *rhs) {
    Node *node = new_node(kind);
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

Node *new_node_num(int value) {
    Node *node = new_node(NODE_NUM);
    node->value = value;
    return node;
}

Node *new_unary(NodeKind kind, Node *lhs) {
    Node *node = new_node(kind);
    node->lhs = lhs;
    return node;
}

Node *new_var(Var *var) {
    Node *node = new_node(NODE_VAR);
    node->var = var;
    return node;
}

Var *find_var(Token *tkn) {
    for (Var *l = locals; l; l=l->next) {
        if (
            strlen(l->name) == tkn->len &&
            !memcmp(tkn->string, l->name, tkn->len)
        ) {
            return l;
        }
    }
    return NULL;
}

Var *push_var(char *name) {
    Var *var = calloc(1, sizeof(Var));
    var->name = name;
    var->next = locals;
    locals = var;
    return var;
}

// program = stmt*
Program *program() {
    locals = NULL;
    // 連結リストで作成していく Node の初期化
    Node head;
    head.next = NULL;
    Node *current_node = &head;

    while (!at_eof()) {
        current_node->next = stmt();
        current_node = current_node->next;
    }

    Program *program = calloc(1, sizeof(Program));
    program->node = head.next;
    program->var = locals;
    return program;
}

// stmt = expr ";" | "return" expr ";"
Node *stmt() {
    if (consume("return")) {
        Node *node = new_unary(NODE_RETURN, expr());
        expect(";");
        return node;
    }

    // Node *node = expr();
    Node *node = new_unary(NODE_EXPR_STMT, expr());
    expect(";");
    return node;
}

// expr = assign
Node *expr() { return assign(); }

// assign = equality ("=" assign)?
Node *assign() {
    Node *node = equality();
    if (consume("=")) {
        node = new_node_binary(NODE_ASSIGN, node, assign());
    }
    return node;
}

// equality = relational ("==" relational | "!=" relational)*
Node *equality() {
    Node *node = relational();

    for (;;) {
        if (consume("==")) {
            node = new_node_binary(NODE_EQ, node, relational());
        } else if (consume("!=")) {
            node = new_node_binary(NODE_NE, node, relational());
        } else {
            return node; // 最終的に生成される node
        }
    }
}

// add ("<" add | "<=" add | ">" add | ">=" add )*
Node *relational(){
    Node *node = add();

    for (;;) {
        if (consume("<")) {
            node = new_node_binary(NODE_LT, node, add());
        } else if (consume("<=")) {
            node = new_node_binary(NODE_LE, node, add());
        } else if (consume(">")) {
            node = new_node_binary(NODE_LT, add(), node);
        } else if (consume(">=")) {
            node = new_node_binary(NODE_LE, add(), node);
        } else {
            return node; // 最終的に生成される node
        }
    }
}

// add ("<" add | "<=" add | ">" add | ">=" add)*
Node *add(){
    Node *node = mul();

    for (;;) {
        if (consume("+")) {
            node = new_node_binary(NODE_ADD, node, mul());
        } else if (consume("-")) {
            node = new_node_binary(NODE_SUB, node, mul());
        } else {
            return node; // 最終的に生成される node
        }
    }
}

// unary ("*" unary | "/" unary)*
Node *mul() {
    Node *node = unary();

    for (;;) {
        if (consume("*")) {
            node = new_node_binary(NODE_MUL, node, unary());
        } else if (consume("/")) {
            node = new_node_binary(NODE_DIV, node, unary());
        } else {
            return node; // 最終的に生成される node
        }
    }
}

// unary = ("+" | "-")? unary | primary
Node *unary() {
    if (consume("+")) {
        return unary();
    }
    if (consume("-")) {
        return new_node_binary(NODE_SUB, new_node_num(0), unary());
    }
    return primary();
}

// num | ident | "(" expr ")"
Node *primary() {
    if (consume("(")) {
        Node *node = expr();
        expect(")");
        return node; // この時点で token は ) の次の token を指している
    }

    // 変数に関する処理
    Token *tkn = consume_ident();
    if (tkn) {
        // token が存在にあるか確認する処理
        Var *var = find_var(tkn);
        if (!var) {
            // locals に var を積む処理
            var = push_var(strndup(tkn->string, tkn->len));
        }
        return new_var(var);
    }

    return new_node_num(expect_number());  // この時点で token は数字の次の token を指している
}
