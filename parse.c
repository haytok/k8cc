#include "k8cc.h"

VarList *locals;
VarList *globals;
VarList *scope;

// 関数の循環参照のためのプロトタイプ宣言
Type *basetype();
Type *read_type_suffix(Type *base);
Function *function();
Node *declaration();
Node *stmt();
Node *expr();
Node *assign();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *unary();
Node *postfix();
Node *primary();
// struct member に関する処理
Member *struct_member();
Type *struct_decl();

// 構文解析
Node *new_node(NodeKind kind, Token *tkn) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = kind;
    node->token = tkn;
    return node;
}

Node *new_node_binary(NodeKind kind, Node *lhs, Node *rhs, Token *tkn) {
    Node *node = new_node(kind, tkn);
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

Node *new_node_num(int value, Token *tkn) {
    Node *node = new_node(NODE_NUM, tkn);
    node->value = value;
    return node;
}

Node *new_unary(NodeKind kind, Node *lhs, Token *tkn) {
    Node *node = new_node(kind, tkn);
    node->lhs = lhs;
    return node;
}

Node *new_var(Var *var, Token *tkn) {
    Node *node = new_node(NODE_VAR, tkn);
    node->var = var;
    return node;
}

Var *find_var(Token *tkn) {
    for (VarList *vl = scope; vl; vl = vl->next) {
        Var *var = vl->var;
        if (
            strlen(var->name) == tkn->len &&
            !memcmp(tkn->string, var->name, tkn->len)
        ) {
            return var;
        }
    }
    return NULL;
}

Var *push_var(char *name, Type *ty, bool is_local) {
    Var *var = calloc(1, sizeof(Var));
    var->name = name;
    var->ty = ty;
    var->is_local = is_local;

    VarList *vl = calloc(1, sizeof(VarList));
    vl->var = var;

    if (is_local) {
        vl->next = locals;
        locals = vl;
    } else {
        vl->next = globals;
        globals = vl;
    }

    VarList *sc = calloc(1, sizeof(VarList));
    sc->var = var;
    sc->next = scope;
    scope = sc;

    return var;
}

// global-var = basetype ident ("[" num "]")* ";"
void global_var() {
    Type *base = basetype();
    char *ident = expect_ident();
    Type *ty = read_type_suffix(base);
    push_var(ident, ty, false);
    expect(";");
}

char *new_label() {
    static int count = 0;
    char buf[20];
    sprintf(buf, ".LC%d", count++);
    return strndup(buf, 20);
}

bool is_type_name() {
    return peek("int") || peek("char") || peek("struct");
}

// struct に関する処理 ex) struct {int a; int b;} x;
// struct-decl = "struct" "{" struct-member "}"
Type *struct_decl() {
    if (consume("struct")) {
        if (consume("{")) {
            Member head;
            head.next = NULL;
            Member *current_member = &head;
            while (!consume("}")) {
                current_member->next = struct_member();
                current_member = current_member->next;
            }
            Type *ty = struct_type();
            ty->members = head.next;
            int offset = 0;
            for (Member *m = ty->members; m; m = m->next) {
                m->offset = offset;
                offset += size_of(m->ty);
            }
            return ty;
        }
    }
}

// struct member に関する処理 ex) struct {int a; int b;} x;
// struct-member = (basetype ident "[" num "]" ";")*
Member *struct_member() {
    // 構造体の member を追加する処理
    Member *m = calloc(1, sizeof(Member));
    Type *base = basetype();
    m->name = expect_ident();
    m->ty = read_type_suffix(base);
    expect(";");
    return m;
}

// Type に関する処理
// basetype = ("int" | "char" | struct-decl) "*"*
Type *basetype() {
    if (!is_type_name()) {
        error_token(token, "Invalid token.");
    }

    Type *ty;
    if (consume("char")) {
        ty = char_type();
    } else if (consume("int")) {
        ty = int_type();
    } else if (peek("struct")) {
        ty = struct_decl();
    }

    while(consume("*")) {
        ty = pointer_to(ty);
    }
    return ty;
}

// function_args = "(" (assign (, assign)*)? ")"
Node *function_args() {
    if (consume(")")) {
        return NULL;
    }

    // 引数が最低一つは存在している前提
    Node *head = assign();
    Node *current_node = head;

    while (consume(",")) {
        // node->args に付け足す処理
        current_node->next = assign();
        current_node = current_node->next;
    }
    expect(")");

    return head;
}

// ident ("[" num "]")* のケースを想定
// ex) int x[2]; をコンパイルできるように実装する
Type *read_type_suffix(Type *base) {
    if (!consume("[")) {
        return base;
    }
    int array_size = expect_number();
    expect("]");
    Type *ty = read_type_suffix(base);
    return array_of(ty, array_size);
}

// basetype ident ("[" num "]")* のケースをコンパイルできるように修正
VarList *read_function_param() {
    VarList *vl = calloc(1, sizeof(VarList));
    Type *base = basetype();
    char *ident = expect_ident();
    Type *ty = read_type_suffix(base);
    vl->var = push_var(ident, ty, true);
    return vl;
}

// params = ident ("," ident)*
VarList *read_function_params() {
    if (consume(")")) {
        return NULL;
    }

    // 初期化
    // locals に追加しつつ、独自に current_var_list に Var 型のオブジェクトを繋げていっている。
    // なので、f->params を for 文で回しても local 変数は現れず、引数のみ現れる。
    VarList *head = read_function_param();
    VarList *current_var_list = head;

    while (!consume(")")) {
        expect(",");
        current_var_list->next = read_function_param();
        current_var_list = current_var_list->next;
    }

    return head;
}

// int x; int main() {} を区別する
bool is_function() {
    // 後々の処理で動かした token を元に戻すための変数
    Token *tkn = token;
    basetype();
    bool isFunc = consume_ident() && consume("(");
    token = tkn;
    return isFunc;
}

// program = (global-var | function)*
Program *program() {
    Function head;
    head.next = NULL;
    Function *current_function = &head;
    globals = NULL;

    while (!at_eof()) {
        if (is_function()) {
            current_function->next = function();
            current_function = current_function->next;
        } else {
            global_var();
        }
    }

    Program *prog = calloc(1, sizeof(Program));
    prog->functions = head.next;
    prog->globals = globals;

    return prog;
}

// function = basetype ident "(" params? ")" "{" stmt* "}"
// params = param ("," param)*
// param = basetype ident
Function *function() {
    locals = NULL;

    Function *function = calloc(1, sizeof(Function));
    basetype();
    function->function_name = expect_ident();
    expect("(");
    // params の処理
    // read_function_params 関数で引数を locals に付け足す実装になっている。
    function->params = read_function_params();
    expect("{");

    // stmt の処理
    Node head;
    head.next = NULL;
    Node *current_node = &head;
    while (!consume("}")) {
        current_node->next = stmt();
        current_node = current_node->next;
    }

    function->node = head.next;
    function->locals = locals;

    return function;
}

// 変数の宣言 ex) int a; int b = 10; などが該当
// declaration = basetype ident ("[" num "]")* ("=" expr) ";"
Node *declaration() {
    Token *tkn = token;
    Type *base = basetype();
    char *ident = expect_ident();
    Type *ty = read_type_suffix(base);
    // Array かどうかの処理を呼び出したい
    Var *var = push_var(ident, ty, true);  // locals には未定義変数が積まれている

    if (consume(";")) {
        return new_node(NODE_NULL, tkn);
    }

    Node *lhs = new_var(var, tkn);
    expect("=");
    Node *rhs = expr();
    Node *node = new_node_binary(NODE_ASSIGN, lhs, rhs, tkn);
    expect(";");
    return new_unary(NODE_EXPR_STMT, node, tkn); // stmt 関数内で呼び出される
}

Node *read_expr_stmt() {
  Token *tkn = token;
  return new_unary(NODE_EXPR_STMT, expr(), tkn);
}

// stmt = expr ";"
// | "{" stmt* "}"
// | "if" "(" expr ")" stmt ("else" stmt)?
// | "while" "(" expr ")" stmt
// | "for" "(" expr? ";" expr? ";" expr? ")" stmt
// | declaration // 今回の実装では int のみを考慮
// | "return" expr ";"
Node *stmt() {
    Token *tkn;
    if (tkn = consume("return")) {
        Node *node = new_unary(NODE_RETURN, expr(), tkn);
        expect(";");
        return node;
    }

    if (tkn = consume("{")) {
        Node head;
        head.next = NULL;
        Node *current_node = &head;

        VarList *sc = scope;
        while (!consume("}")) {
            current_node->next = stmt();
            current_node = current_node->next;
        }
        scope = sc;

        Node *node = new_node(NODE_BLOCK, tkn);
        node->body = head.next;
        return node;
    }

    if (tkn = consume("if")) {
        if (consume("(")) {
            Node *node = new_node(NODE_IF, tkn);
            node->condition = expr();
            expect(")");
            node->then = stmt();
            if (consume("else")) {
                node->els = stmt();
            }
            return node;
        }
    }

    if (tkn = consume("while")) {
        if (consume("(")) {
            Node *node = new_node(NODE_WHILE, tkn);
            node->condition = expr();
            expect(")");
            node->then = stmt();
            return node;
        }
    }

    if (tkn = consume("for")) {
        if (consume("(")) {
            Node *node = new_node(NODE_FOR, tkn);
            if (!consume(";")) {
                node->init = read_expr_stmt();
                expect(";");
            }
            if (!consume(";")) {
                node->condition = expr();
                expect(";");
            }
            if (!consume(")")) {
                node->inc = read_expr_stmt();
                expect(")");
            }
            node->then = stmt();
            return node;
        }
    }

    if (is_type_name()) {
        return declaration();
    }

    // Node *node = expr();
    Node *node = new_unary(NODE_EXPR_STMT, expr(), tkn);
    expect(";");
    return node;
}

// expr = assign
Node *expr() { return assign(); }

// assign = equality ("=" assign)?
Node *assign() {
    Node *node = equality();
    Token *tkn;
    if (tkn = consume("=")) {
        node = new_node_binary(NODE_ASSIGN, node, assign(), tkn);
    }
    return node;
}

// equality = relational ("==" relational | "!=" relational)*
Node *equality() {
    Node *node = relational();
    Token *tkn;

    for (;;) {
        if (tkn = consume("==")) {
            node = new_node_binary(NODE_EQ, node, relational(), tkn);
        } else if (tkn = consume("!=")) {
            node = new_node_binary(NODE_NE, node, relational(), tkn);
        } else {
            return node; // 最終的に生成される node
        }
    }
}

// add ("<" add | "<=" add | ">" add | ">=" add )*
Node *relational() {
    Node *node = add();
    Token *tkn;

    for (;;) {
        if (tkn = consume("<")) {
            node = new_node_binary(NODE_LT, node, add(), tkn);
        } else if (tkn = consume("<=")) {
            node = new_node_binary(NODE_LE, node, add(), tkn);
        } else if (tkn = consume(">")) {
            node = new_node_binary(NODE_LT, add(), node, tkn);
        } else if (tkn = consume(">=")) {
            node = new_node_binary(NODE_LE, add(), node, tkn);
        } else {
            return node; // 最終的に生成される node
        }
    }
}

// add ("<" add | "<=" add | ">" add | ">=" add)*
Node *add() {
    Node *node = mul();
    Token *tkn;

    for (;;) {
        if (tkn = consume("+")) {
            node = new_node_binary(NODE_ADD, node, mul(), tkn);
        } else if (tkn = consume("-")) {
            node = new_node_binary(NODE_SUB, node, mul(), tkn);
        } else {
            return node; // 最終的に生成される node
        }
    }
}

// unary ("*" unary | "/" unary)*
Node *mul() {
    Node *node = unary();
    Token *tkn;

    for (;;) {
        if (tkn = consume("*")) {
            node = new_node_binary(NODE_MUL, node, unary(), tkn);
        } else if (tkn = consume("/")) {
            node = new_node_binary(NODE_DIV, node, unary(), tkn);
        } else {
            return node; // 最終的に生成される node
        }
    }
}

// unary = ("*" | "&" | "+" | "-")? unary
// | postfix // x[0] = 1; のようなケースを想定して文法を追加
Node *unary() {
    Token *tkn;
    if (tkn = consume("*")) {
        return new_unary(NODE_DEREF, unary(), tkn);
    }
    if (tkn = consume("&")) {
        return new_unary(NODE_ADDRESS, unary(), tkn);
    }
    if (consume("+")) {
        return primary();
    }
    if (tkn = consume("-")) {
        return new_node_binary(NODE_SUB, new_node_num(0, tkn), unary(), tkn);
    }
    return postfix();
}

// struct に関する処理も考慮する必要がある。 ex) x.a=1; x.b=2; x.a;
// postfix = primary ( "[" expr "]" | "." ident)*
Node *postfix() {
    Node *node = primary();
    Token *tkn;

    for (;;) {
        if (tkn = consume("[")) {
            // x[y] is short for *(x+y).
            Node *exp = new_node_binary(NODE_ADD, node, expr(), tkn);
            expect("]");
            node = new_unary(NODE_DEREF, exp, tkn);
            continue;
        }
        if (tkn = consume(".")) {
            node = new_unary(NODE_MEMBER, node, tkn);
            node->member_name = expect_ident();
            continue;
        }

        return node;
    }
}

// stmt-expr = "(" "{" stmt stmt* "}" ")"
//
// Statement expression is a GNU C extension.
Node *stmt_expr(Token *tkn) {
    VarList *sc = scope;

    Node *node = new_node(NODE_STMT_EXPR, tkn);
    node->body = stmt();
    Node *current_node = node->body;

    while (!consume("}")) {
        current_node->next = stmt();
        current_node = current_node->next;
    }
    expect(")");

    scope = sc;

    if (current_node->kind != NODE_EXPR_STMT)
        error_token(current_node->token, "stmt expr returning void is not supported");

    *current_node = *current_node->lhs; // 処理の意味がわからん。
    return node;
}

// stmt-expr-tail
// function_args = "(" (assign (, assign)*)? ")"
// primary = "(" "{" stmt-expr-tail
// | num
// | ident function_args?
// | "(" expr ")"
// | "sizeof" unary
// | str
Node *primary() {
    Token *tkn;
    if (tkn = consume("(")) {
        if (consume("{")) {
            return stmt_expr(tkn);
        }

        Node *node = expr();
        expect(")");
        return node; // この時点で token は ) の次の token を指している
    }


    if (tkn = consume("sizeof")) {
        return new_unary(NODE_SIZEOF, unary(), tkn);
    }

    // 変数に関する処理
    tkn = consume_ident();
    if (tkn) {
        // トークンが関数で使われているかを検証
        if (consume("(")) {
            Node *node = new_node(NODE_FUNCALL, tkn);
            node->function_name = strndup(tkn->string, tkn->len);
            node->args = function_args();
            return node;
        }
        // token が存在にあるか確認する処理
        Var *var = find_var(tkn);
        if (!var) {
            // locals に変数名が積まれていない時 (変数が宣言されていない時) は、エラーを吐く
            error_token(tkn, "undefined variable.");
        }
        return new_var(var, tkn);
    }

    tkn = token;
    // 文字列リテラルの処理
    if (tkn->kind == TK_STR) {
        token = token->next;
        Type *ty = array_of(char_type(), tkn->cont_len);
        Var *var = push_var(new_label(), ty, false);
        var->contents = tkn->contents;
        var->cont_len = tkn->cont_len;
        return new_var(var, tkn);
    }
    if (tkn->kind != TK_NUM) {
        error_token(tkn, "Invalid token.");
    }

    return new_node_num(expect_number(), tkn);  // この時点で token は数字の次の token を指している
}
