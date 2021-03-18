# メモ
- fprintf と stderr を使う理由
  - [画面に文字を出力 ～ 標準出力と標準エラーへの出力](https://c.keicode.com/lang/stdout-stderr.php)
- [*argv[] **argvとはなんぞや？](https://qiita.com/violeteverganden/items/937f4d310e4f99d28cc6)
- [atoi 関数について](http://hitorilife.com/atoi.php)
- [strtol 関数] (http://www.c-tipsref.com/reference/stdlib/strtol.html)
- [strchr](http://www.c-tipsref.com/reference/string/strchr.html)
  - argument: const char *s, int c
  - strchr 関数は s が指す文字列の中で c が最初に現れる位置を探します．(終端ナル文字は文字列の一部とみなされます．) 探索中は c は char 型として解釈されます．
- [memcmp 関数について](https://bituse.info/c_func/55)
  - 第一引数, 第二引数は比較対象メモリのポインタ, 第三引数は比較サイズ
  - strcmp と違って比較サイズを指定できる。
- [strcmp 関数について](http://hitorilife.com/strcmp.php)
   - int strcmp( const char *str1 , const char *str2 );
   - str1とstr2が等しいならば0、str1 > str2ならば正の値、str1 < str2ならば負の値を返す。
- 文字を引数とする時は char op で良いが、文字列を引数とする時は char *op でポインタを引数として渡すのが良いはず。
- `man strcpy` で引数などを調べることができる。

- まず、テストケースを実装すると全体像が把握しやすいかも。
- 方針がなんとなく立てば、とりあえず実装してみて、前半の取っていないといけないテストが通らない時は、前回の実装との整合性が取れておらずバグが生じているので、その調査に取りかかる。

- 複数文字のローカル変数を用いたテストケースのアセンブリ

```asm
.intel_syntax noprefix
.global main
main:
プロローグ
  push rbp
  mov rbp, rsp
変数のサイズ文だけスタックポインタを下げる
  sub rsp, 16
メイン処理に入る foo123=3; 
case NODE_ASSIGN: の処理が始まる
  mov rax, rbp
  sub rax, 16
  push rax
この時点でスタックの先頭に変数のアドレスが積まれている
  push 3
関数以外の処理が走る。
  pop rdi
  pop rax
  mov [rax], rdi
  push rdi
一つのステートメントが終了
  add rsp, 8
bar=5; の処理が始まる
  mov rax, rbp
  sub rax, 8
  push rax
  push 5
  pop rdi
  pop rax
  mov [rax], rdi
  push rdi
  add rsp, 8
  mov rax, rbp
  sub rax, 16
  push rax
  pop rax
  mov rax, [rax]
  push rax
  mov rax, rbp
  sub rax, 8
  push rax
  pop rax
  mov rax, [rax]
  push rax
  pop rdi
  pop rax
  add rax, rdi
  push rax
  pop rax
  jmp .Lreturn
エピローグ
.Lreturn:
  mov rsp, rbp
  pop rbp
  ret
```

- 文字列の配列の長さを動的に取得するには以下のようなコードを書けば良い。

```c
char *keyword[] = {"return", "if", "else"};
int keyword_size = sizeof(keyword) / sizeof(*keyword);
```

- 以下のテストケースにおけるコンパイラで吐き出したアセンブリ

```c
assert 13 "return add(add(3, 5), 5);"
```

```asm
.intel_syntax noprefix
.global main
main:
  push rbp
  mov rbp, rsp
  sub rsp, 0
  # メインの処理
  push 3
  push 5
  pop rsi
  pop rdi
  call add
  push rax
  push 5
  pop rsi
  pop rdi
  call add
  push rax
  pop rax
  jmp .Lreturn
.Lreturn:
  mov rsp, rbp
  pop rbp
  ret
```

- 疑問
  - 関数の定義のコンパイラを実装する段階で以下の実装の違いがわからんかった。

```c
Function *program() {
    Function head;
    head.next = NULL;
    Function *current_function = &head;

    while (!at_eof()) {
        current_function->next = function();
        current_function = current_function->next;
    }

    return head.next;
}

Function *program() {
    Function *head;
    head->next = NULL;
    Function *current_function = head;

    while (!at_eof()) {
        current_function->next = function();
        current_function = current_function->next;
    }

    return head;
}
```
- 関数のアセンブラのラベルについて
  - .global は関数毎に持っていて、ret も同様である。

```asm
.intel_syntax noprefix

# main 関数の定義
.global main
main:
# プロローグ
  push rbp
  mov rbp, rsp
  sub rsp, 0

  mov rax, rsp
  and rax, 15
  jnz .Lcall1
  mov rax, 0
  call ret32
  jmp .Lend1
  .Lcall1:
  sub rsp, 8
  mov rax, 0
  call ret32
  add rsp, 8
  .Lend1:
  push rax
  pop rax
  jmp .Lreturn.main
# エピローグ
.Lreturn.main:
  mov rsp, rbp
  pop rbp
  ret

# ret32 関数の定義
.global ret32
ret32:
# プロローグ
  push rbp
  mov rbp, rsp
  sub rsp, 0

  push 32
  pop rax
  jmp .Lreturn.ret32
# エピローグ
.Lreturn.ret32:
  mov rsp, rbp
  pop rbp
  ret
```

- 適当にアセンブラを書いて実装のイメージを湧かしても良かったかもしれない。具体から抽象の書き起こし。

- `assert 7 "main() { return add2(3,4); } add2(x,y) { return x+y; }"`

```asm
.intel_syntax noprefix
.global main
main:
# プロローグ
  push rbp
  mov rbp, rsp
  sub rsp, 0

# main 関数で呼び出した add2 関数の引数をレジスタに格納している。
  push 3
  push 4
  pop rsi
  pop rdi
  mov rax, rsp
  and rax, 15
  jnz .Lcall0
  mov rax, 0
  call add2 # call
  jmp .Lend0
  .Lcall0:
  sub rsp, 8
  mov rax, 0
  call add2 # call
  add rsp, 8
  .Lend0:
  push rax
  pop rax
  jmp .Lreturn.main
.Lreturn.main:
  mov rsp, rbp
  pop rbp
  ret
.global add2
add2:
# プロローグ
  push rbp
  mov rbp, rsp
  sub rsp, 16

  mov [rbp-16], rdi # 第一引数がベースポインタより二つ下のスタックにあるアドレスがの先コピー
  mov [rbp-8], rsi # 第二引数がベースポインタより一つ下のスタックにあるアドレスがの先コピー

# for (Node *n = f->node; n; n = n->next) {gen(n);} の処理が走る
# case NODE_VAR:
# gen_lval が吐き出すアセンブラ
  mov rax, rbp
  sub rax, 16
  push rax
  pop rax
  mov rax, [rax]
  push rax

  mov rax, rbp
  sub rax, 8
  push rax
  pop rax
  mov rax, [rax]
  push rax

  pop rdi
  pop rax

  add rax, rdi # +
  push rax
# return
  pop rax
  jmp .Lreturn.add2
.Lreturn.add2:
  mov rsp, rbp
  pop rbp
  ret
```

- codegen.c で以下の処理が呼び出せる理由がわからない。main 関数では、f->locals->var->offset に 8, 16 を代入しているが、以下の f->params->var->offset には値が入ってないと思った。参照が同一だから、値が入っているのかも。

```c
int i = 0;
for (VarList *p = f->params; p; p = p->next) {
    Var *v = p->var;
    printf("  mov [rbp-%d], %s\n", v->offset, arg_register[i++]);
}
```

- `assert 8 "main() { return add2(3,4); } add2(x,y) { a=1; return a+x+y; }"`

```asm
.intel_syntax noprefix
.global main
main:
  push rbp
  mov rbp, rsp
  sub rsp, 0
  push 3
  push 4
  pop rsi
  pop rdi
  mov rax, rsp
  and rax, 15
  jnz .Lcall0
  mov rax, 0
  call add2
  jmp .Lend0
  .Lcall0:
  sub rsp, 8
  mov rax, 0
  call add2
  add rsp, 8
  .Lend0:
  push rax
  pop rax
  jmp .Lreturn.main
.Lreturn.main:
  mov rsp, rbp
  pop rbp
  ret
.global add2
add2:
  push rbp
  mov rbp, rsp
  sub rsp, 24
  mov [rbp-24], rdi
  mov [rbp-16], rsi
  mov rax, rbp
  sub rax, 8
  push rax
  push 1
  pop rdi
  pop rax
  mov [rax], rdi
  push rdi
  add rsp, 8
  mov rax, rbp
  sub rax, 8
  push rax
  pop rax
  mov rax, [rax]
  push rax
  mov rax, rbp
  sub rax, 24
  push rax
  pop rax
  mov rax, [rax]
  push rax
  pop rdi
  pop rax
  add rax, rdi
  push rax
  mov rax, rbp
  sub rax, 16
  push rax
  pop rax
  mov rax, [rax]
  push rax
  pop rdi
  pop rax
  add rax, rdi
  push rax
  pop rax
  jmp .Lreturn.add2
.Lreturn.add2:
  mov rsp, rbp
  pop rbp
  ret
```

- エラー文を表示する際に用いる変数や関数
- `fmt` (format)
- `stderr`

- `va_list`
- `va_start`
- printf などの関数では、第二引数以降の可変長引数にアクセスする必要がある。この可変長引数リストにアクセスするには、`va_list` 型の変数に引数リストのポインタを指定する。それが以下の処理になる。

```c
void error_at(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
}
```

- `fprintf`
- ストリームにフォーマットされた文字列を出力する。
- 例えば `fprintf(stderr, "\n");` のケースだと標準エラー出力に "\n" を書き込むという意味である。定義は以下である。
```c
int fprintf(FILE *stream, const char *format, ...);
```

- `vfprintf`
- 定義は以下である。`fprintf` の可変長引数リストを `va_list arg` に置き換えたものである。
```c
int vfprintf(FILE *stream, const char *format, va_list arg);
```

- アドレスを表す単項 `&` とアドレスを参照する `*` を実装していく。
- `&x` は、変数 x のアドレスを単なる整数として返す演算。
- `*x` は、x の値をアドレスとみなして、そのアドレスから値を読み込む演算。

- `main() { x=3; y=&x; *y=5; return x; }`
```asm
.intel_syntax noprefix
.global main
main:
  push rbp
  mov rbp, rsp
  sub rsp, 16

  mov rax, rbp
  sub rax, 16
  push rax

# x = 3
  push 3
  pop rdi
  pop rax
  mov [rax], rdi
  push rdi
  add rsp, 8

# y = &x;
  mov rax, rbp
  sub rax, 8
  push rax

  mov rax, rbp
  sub rax, 16
  push rax
  pop rdi
  pop rax
  mov [rax], rdi
  push rdi
  add rsp, 8

# *y = 5
# gen_lval の処理
  mov rax, rbp
  sub rax, 8
  push rax

  pop rax
  mov rax, [rax]
  push rax
# case 文に書か無いと呼び出されない処理

  push 5
# case NODE_ASSIGN の残りの項
  pop rdi
  pop rax
  mov [rax], rdi
  push rdi
  add rsp, 8
# return 文に入る
  mov rax, rbp
  sub rax, 16
  push rax
  pop rax
  mov rax, [rax]
  push rax
  pop rax
  jmp .Lreturn.main
.Lreturn.main:
  mov rsp, rbp
  pop rbp
  ret
```

- ebp - 8 の値は、スタック上の位置を表していて、その値を [] で囲うことでそのスタックの位置に格納されている値を取得できる。*x のケースでは、x にアドレスが格納されているので [ebp - 8] の値はアドレスになる。その値が、DEREF (*) のケースでは、 gen 関数が呼び出される前にスタックのトップに積まれている。

- Annotate AST nodes with types の commit について
  - 資料のステップ 18 の図にあるように一番末端の型は int 型しか現状は取り扱っていない。
  - 悔しいけど、完全には理解できなかった。
  - 特に type.c の以下の実装が完全に理解できなかった。

```c
case ND_ADD:
    if (node->rhs->ty->kind == TY_PTR) {
    Node *tmp = node->lhs;
    node->lhs = node->rhs;
    node->rhs = tmp;
  }
  if (node->rhs->ty->kind == TY_PTR)
    error_tok(node->tok, "invalid pointer arithmetic operands");
  node->ty = node->lhs->ty;
  return;
case ND_SUB:
  if (node->rhs->ty->kind == TY_PTR)
    error_tok(node->tok, "invalid pointer arithmetic operands");
  node->ty = node->lhs->ty;
  return;
```

- global 変数と local 変数について
- 今まではすべての変数がスタックにあったはずなので、変数の読み書きはRBP（ベースポインタ）からの相対で行なっていました。グローバル変数はスタック上にある値ではなく、メモリ上の固定の位置にある値なので、そのアドレスに直接アクセスするようにコンパイルします。
- データセグメントとテキストセグメントの話は資料の実行ファイルの構造の箇所で説明があった。グローバル変数に関数の結果を代入できない話がリンカとコンパイラの文脈で説明されていた、
- `test.sh` の gcc のオプションで `-static` を付けていないと、以下のエラーが出た。

```bash
/usr/bin/ld: /tmp/ccg7ng8r.o: relocation R_X86_64_32S against `.data' can not be used when making a PIE object。 -fPIC を付けて再コンパイルしてください。
/usr/bin/ld: 最終リンクに失敗しました: 出力に対応するセクションがありません
collect2: error: ld returned 1 exit status
./test.sh: 行 20: ./tmp: そのようなファイルやディレクトリはありません
int x; int main() { return x; } => 0, but got 127
Makefile:11: recipe for target 'test' failed
make: *** [test] Error 1
```

- [sprintf 書式指定変換した出力を文字列に格納します](http://www9.plala.or.jp/sgwr-t/lib/sprintf.html)
- strndup 関数

- 文字列リテラルのアセンブラの出力形式は資料のデータセグメントの節に記載されている。
