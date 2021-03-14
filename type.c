#include "k8cc.h"

Type *int_type() {
    Type *ty = calloc(1, sizeof(Type));
    ty->kind = TYPE_INT;
    return ty;
}

// 新しい Type オブジェクトに次のノードの Type オブジェクトを
//追加したオブジェクトを返す関数
Type *pointer_to(Type *base) {
    Type *ty = calloc(1, sizeof(Type));
    ty->kind = TYPE_PTR;
    ty->base = base;
    return ty;
}

void visit(Node *node) {
    if (!node) {
        return;
    }

    // Node オブジェクトが抱える Node オブジェクトに処理を回す
    for (Node *n = node->lhs; n; n = n->next) {
        visit(n);
    }
    for (Node *n = node->rhs; n; n = n->next) {
        visit(n);
    }
    for (Node *n = node->condition; n; n = n->next) {
        visit(n);
    }
    for (Node *n = node->then; n; n = n->next) {
        visit(n);
    }
    for (Node *n = node->els; n; n = n->next) {
        visit(n);
    }
    for (Node *n = node->init; n; n = n->next) {
        visit(n);
    }
    for (Node *n = node->inc; n; n = n->next) {
        visit(n);
    }

    for (Node *n = node->body; n; n = n->next) {
        visit(n);
    }
    for (Node *n = node->args; n; n = n->next) {
        visit(n);
    }

    switch (node->kind) {
        case NODE_MUL:
        case NODE_DIV:
        case NODE_EQ:
        case NODE_NE:
        case NODE_LT:
        case NODE_LE:
        case NODE_VAR:
        case NODE_FUNCALL:
        case NODE_NUM: {
            node->type = int_type();
            return;
        }
        case NODE_ADD: {
            if (node->rhs->type->kind == TYPE_PTR) {
                Node *tmp = node->rhs;
                node->rhs = node->lhs;
                node->lhs = tmp;
            }
            if (node->rhs->type->kind == TYPE_PTR) {
                error_token(node->token, "invalid pointer arithmetic operands");
            }
            node->type = node->lhs->type;
            return;
        }
        case NODE_SUB: {
            if (node->rhs->type->kind == TYPE_PTR) {
                error_token(node->token, "invalid pointer arithmetic operands");
            }
            node->type = node->lhs->type;
            return;
        }
        case NODE_ASSIGN: {
            node->type = node->lhs->type;
            return;
        }
        // *
        case NODE_DEREF: {
            // イマイチ理解できていない。
            if (node->lhs->type->kind == TYPE_PTR) {
                node->type = node->lhs->type->base;
            } else {
                node->type = int_type();
            }
            return;
        }
        // &
        case NODE_ADDRESS: {
            node->type = pointer_to(node->lhs->type); // イマイチ理解できていない。
            return;
        }

    }
}

void add_type(Function *prog) {
    for (Function *f = prog; f; f = f->next) {
        for (Node *n = f->node; n; n = n->next) {
            visit(n);
        }
    }
}
