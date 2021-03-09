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
