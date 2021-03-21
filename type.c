#include "k8cc.h"

Type *new_type(TypeKind kind) {
    Type *ty = calloc(1, sizeof(Type));
    ty->kind = kind;
    return ty;
}

Type *int_type() {
    return new_type(TYPE_INT);
}

Type *char_type() {
    return new_type(TYPE_CHAR);
}

Type *struct_type() {
    return new_type(TYPE_STRUCT);
}

// 新しい Type オブジェクトに次のノードの Type オブジェクトを
//追加したオブジェクトを返す関数
Type *pointer_to(Type *base) {
    Type *ty = new_type(TYPE_PTR);
    ty->base = base;
    return ty;
}

int size_of(Type *ty) {
    switch(ty->kind) {
        case TYPE_CHAR:
            return 1;
        case TYPE_INT:
        case TYPE_PTR: {
            return 8;
        }
        case TYPE_ARRAY: {
            return size_of(ty->base) * ty->array_size;
        }
        default: {
            Member *m = ty->members;
            while (m->next) {
                m = m->next;
            }
            return m->offset + size_of(m->ty);
        }
    }
}

Member *find_member(Type *ty, char *name) {
    for (Member *m = ty->members; m; m = m->next) {
        if (!strcmp(m->name, name)) {
            return m;
        }
    }
    return NULL;
}

// Array
Type *array_of(Type *base, int array_size) {
    Type *ty = new_type(TYPE_ARRAY);
    ty->base = base;
    ty->array_size = array_size;
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
        case NODE_FUNCALL:
        case NODE_NUM: {
            node->type = int_type();
            return;
        }
        case NODE_VAR: {
            node->type = node->var->ty;
            return;
        }
        case NODE_ADD: {
            if (node->rhs->type->base) {
                Node *tmp = node->rhs;
                node->rhs = node->lhs;
                node->lhs = tmp;
            }
            if (node->rhs->type->base) {
                error_token(node->token, "invalid pointer arithmetic operands");
            }
            node->type = node->lhs->type;
            return;
        }
        case NODE_SUB: {
            if (node->rhs->type->base) {
                error_token(node->token, "invalid pointer arithmetic operands");
            }
            node->type = node->lhs->type;
            return;
        }
        case NODE_ASSIGN: {
            node->type = node->lhs->type;
            return;
        }
        case NODE_MEMBER: {
            if (node->lhs->type->kind != TYPE_STRUCT)
                error_token(node->token, "not a struct");
            node->member = find_member(node->lhs->type, node->member_name);
            if (!node->member)
                error_token(node->token, "specified member does not exist");
            node->type = node->member->ty;
            return;
        }
        // *
        case NODE_DEREF: {
            // イマイチ理解できていない。
            if (!node->lhs->type->base) {
                error_token(node->token, "invalid pointer dereference");
            }
            node->type = node->lhs->type->base;
            return;
        }
        // &
        case NODE_ADDRESS: {
            if (node->lhs->type->kind == TYPE_ARRAY) {
                node->type = pointer_to(node->lhs->type->base);
            } else {
                node->type = pointer_to(node->lhs->type); // イマイチ理解できていない。
            }
            return;
        }
        case NODE_SIZEOF: {
            node->kind = NODE_NUM;
            node->type = int_type();
            node->value = size_of(node->lhs->type);
            node->lhs = NULL;
            return;
        }
        case NODE_STMT_EXPR: {
            Node *last = node->body;
            while (last->next) {
                last = last->next;
            }
            node->type = last->type;
            return;
        }
    }
}

void add_type(Program *prog) {
    for (Function *f = prog->functions; f; f = f->next) {
        for (Node *n = f->node; n; n = n->next) {
            visit(n);
        }
    }
}
