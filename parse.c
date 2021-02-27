#include "k8cc.h"

// 関数の循環参照のためのプロトタイプ宣言
Node *expr();
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

// expr = equality
Node *expr() { return equality(); }

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

// num | "(" expr ")"
Node *primary() {
    if (consume("(")) {
        Node *node = expr();
        expect(")");
        return node; // この時点で token は ) の次の token を指している
    }
    return new_node_num(expect_number()); // この時点で token は数字の次の token を指している
}
