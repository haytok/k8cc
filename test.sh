#!/bin/bash
# 別のオブジェクトファイルを作成して、リンカでリンクさせる。
# 自作コンパイラ側で以下の関数を呼び出す。
cat <<EOF | gcc -xc -c -o tmp2.o -
int ret3() { return 3; }
int ret5() { return 5; }
int add(int x, int y) { return x+y; }
int sub(int x, int y) { return x-y; }
int add6(int a, int b, int c, int d, int e, int f) {
  return a+b+c+d+e+f;
}
EOF

assert() {
    expected="$1"
    input="$2"

    ./k8cc "$input" > tmp.s
    gcc -static -o tmp tmp.s tmp2.o
    ./tmp
    actual="$?"

    if [ "$actual" = "$expected" ]; then
      echo "$input => $actual"
    else
      echo "$input => $expected, but got $actual"
      exit 1
    fi
}

# テストケース
assert 0 "int main() { return 0; }"
assert 42 "int main() { return 42; }"

assert 21 "int main() { return 5+20-4; }"

assert 41 "int main() { return 12 + 34 - 5; }"

assert 47 "int main() { return 5+6*7; }"
assert 15 "int main() { return 5*(9-6); }"
assert 4 "int main() { return (3+5)/2; }"

assert 10 "int main() { return -10+20; }"
assert 10 "int main() { return - -10; }"
assert 10 "int main() { return - - +10; }"

# 比較演算子
assert 0 "int main() { return 0==1; }"
assert 1 "int main() { return 42==42; }"
assert 1 "int main() { return 0!=1; }"
assert 0 "int main() { return 42!=42; }"

assert 1 "int main() { return 0<1; }"
assert 0 "int main() { return 1<1; }"
assert 0 "int main() { return 2<1; }"
assert 1 "int main() { return 0<=1; }"
assert 1 "int main() { return 1<=1; }"
assert 0 "int main() { return 2<=1; }"

assert 1 "int main() { return 1>0; }"
assert 0 "int main() { return 1>1; }"
assert 0 "int main() { return 1>2; }"
assert 1 "int main() { return 1>=0; }"
assert 1 "int main() { return 1>=1; }"
assert 0 "int main() { return 1>=2; }"

# 必ず return 文をつける必要がある
assert 1 "int main() { return 1; }"
# assert 1 "1;" # ダメなテストケース
assert 3 "int main() { 1; 2; return 3; }"
assert 1 "int main() { return 1; 2; 3; }"
assert 3 "int main() { return 3; }"
assert 3 "int main() { 1 + 2 + 3; 2; return 3; }"
# アセンブラの一番初めの ret が評価されてる
assert 2 "int main() { 1; return 2; 3; }"
assert 2 "int main() { 1; return 2; return 3; }"

# 一文字の変数を用いたテストケース
assert 3 "int main() { int a; a=3; return a; }"
assert 8 "int main() { int a; int z; a=3; z=5; return a+z; }"
assert 14 "int main() { int a; int b; a = 3; b = 5 * 6 - 8; return a + b / 2; }"
assert 3 "int main() { int a=3; return a; }"
assert 8 "int main() { int a=3; int z=5; return a+z; }"

# 複数文字のローカル変数を用いたテストケース
assert 3 "int main() { int foo=3; return foo; }"
assert 8 "int main() { int foo123=3; int bar=5; return foo123+bar; }"

# if statement のテストケース
assert 3 "int main() { if (0) return 2; return 3; }"
assert 3 "int main() { if (1-1) return 2; return 3; }"
assert 2 "int main() { if (1) return 2; return 3; }"
assert 2 "int main() { if (2-1) return 2; return 3; }"
assert 3 "int main() { if (0) return 2; else return 3; return 4; }"
assert 2 "int main() { if (1) return 2; else return 3; return 4; }"

# while statement のテストケース
assert 10 "int main() { int i=0; while(i<10) i=i+1; return i; }"

# for statement のテストケース
assert 55 "int main() { int i=0; int j=0; for (i=0; i<=10; i=i+1) j=i+j; return j; }"
assert 3 "int main() { for (;;) return 3; return 5; }"
assert 40 "int main() { int i=0; int j=0; for (i=0; i<=10; i=i+1) if (i>5) j = i+j; return j; }"

# ブロックのテストケース
assert 3 "int main() { {1; {2;} return 3; } }"
assert 55 "int main() { int i=0; int j=0; while(i<=10) {j=i+j; i=i+1;} return j; }"

# 引数が無い関数のテストケース
assert 3 "int main() { return ret3(); }"
assert 5 "int main() { return ret5(); }"

# 引数が 6 個までの関数のテストケース
assert 8 "int main() { return add(3, 5); }"
assert 2 "int main() { return sub(5, 3); }"
assert 21 "int main() { return add6(1, 2, 3, 4, 5, 6); }"
assert 13 "int main() { return add(3 + 5, 5); }"
assert 13 "int main() { return add(add(3, 5), 5); }"

# 関数の定義に対応したテストケース
assert 32 "int main() { return ret32(); } int ret32() { return 32; }"

# 引数が最大 6 個の関数に対応したテストケース
assert 7 "int main() { return add2(3,4); } int add2(int x, int y) { return x+y; }"
assert 6 "int main() { return aaa(3); } int aaa(int x) { return x*2; }"
assert 12 "int main() { return mul2(3,4); } int mul2(int x, int y) { return x*y; }"
assert 1 "int main() { return sub2(4,3); } int sub2(int x, int y) { return x-y; }"
assert 55 "int main() { return fib(9); } int fib(int x) { if (x<=1) return 1; return fib(x-1) + fib(x-2); }"
assert 8 "int main() { return add2(add2(4,3), sub2(4,3)); } int add2(int x, int y) { return x+y; } int sub2(int x, int y) { return x-y; }"
assert 8 "int main() { return add2(3,4); } int add2(int x, int y) { int a=1; return a+x+y; }"

# & と * に対応したテストケース
assert 3 "int main() { int x=3; return *&x; }"
assert 3 "int main() { int x=3; int *y=&x; int **z=&y; return **z; }"
# Annotate AST nodes with types の commit で削除
# assert 5 "main() { x=3; y=5; return *(&x+8); }" # ローカル変数がメモリ上で連続して割り当てられている前提。
# assert 3 "main() { x=3; y=5; return *(&y-8); }"
assert 5 "int main() { int x=3; int *y=&x; *y=5; return x; }"
# assert 7 "main() { x=3; y=5; *(&x+8)=7; return y; }"
# assert 7 "main() { x=3; y=5; *(&y-8)=7; return x; }"

assert 5 "int main() { int x=3; int y=5; return *(&x+1); }"
assert 3 "int main() { int x=3; int y=5; return *(&y-1); }"
assert 7 "int main() { int x=3; int y=5; *(&x+1)=7; return y; }"
assert 7 "int main() { int x=3; int y=5; *(&y-1)=7; return x; }"

# int 型に対応したテストケース
assert 3 "int main() { int x=3; return *&x; }"
assert 3 "int main() { int x=3; int *y=&x; int **z=&y; return **z; }"
assert 5 "int main() { int x=3; int y=5; return *(&x+1); }"
assert 5 "int main() { int x=3; int y=5; return *(1+&x); }"
assert 3 "int main() { int x=3; int y=5; return *(&y-1); }"
assert 5 "int main() { int x=3; int y=5; int *z=&x; return *(z+1); }"
assert 3 "int main() { int x=3; int y=5; int *z=&y; return *(z-1); }"
assert 5 "int main() { int x=3; int *y=&x; *y=5; return x; }"
assert 7 "int main() { int x=3; int y=5; *(&x+1)=7; return y; }"
assert 7 "int main() { int x=3; int y=5; *(&y-1)=7; return x; }"
assert 8 "int main() { int x=3; int y=5; return foo(&x, y); } int foo(int *x, int y) { return *x + y; }"

# Array に対応したテストケース
assert 3 "int main() { int x[2]; int *y=&x; *y=3; return *x; }"

assert 3 "int main() { int x[3]; *x=3; *(x+1)=4; *(x+2)=5; return *x; }"
assert 4 "int main() { int x[3]; *x=3; *(x+1)=4; *(x+2)=5; return *(x+1); }"
assert 5 "int main() { int x[3]; *x=3; *(x+1)=4; *(x+2)=5; return *(x+2); }"

assert 0 "int main() { int x[2][3]; int *y=x; *y=0; return **x; }"
assert 1 "int main() { int x[2][3]; int *y=x; *(y+1)=1; return *(*x+1); }"
assert 2 "int main() { int x[2][3]; int *y=x; *(y+2)=2; return *(*x+2); }"
assert 3 "int main() { int x[2][3]; int *y=x; *(y+3)=3; return **(x+1); }"
assert 4 "int main() { int x[2][3]; int *y=x; *(y+4)=4; return *(*(x+1)+1); }"
assert 5 "int main() { int x[2][3]; int *y=x; *(y+5)=5; return *(*(x+1)+2); }"
assert 6 "int main() { int x[2][3]; int *y=x; *(y+6)=6; return **(x+2); }"

# Array に [] operator でアクセスできるようなテストケース
assert 3 "int main() { int x[3]; *x=3; x[1]=4; x[2]=5; return *x; }"
assert 4 "int main() { int x[3]; *x=3; x[1]=4; x[2]=5; return *(x+1); }"
assert 5 "int main() { int x[3]; *x=3; x[1]=4; x[2]=5; return *(x+2); }"
assert 5 "int main() { int x[3]; *x=3; x[1]=4; x[2]=5; return *(x+2); }"
assert 5 "int main() { int x[3]; *x=3; x[1]=4; 2[x]=5; return *(x+2); }"

assert 0 "int main() { int x[2][3]; int *y=x; y[0]=0; return x[0][0]; }"
assert 1 "int main() { int x[2][3]; int *y=x; y[1]=1; return x[0][1]; }"
assert 2 "int main() { int x[2][3]; int *y=x; y[2]=2; return x[0][2]; }"
assert 3 "int main() { int x[2][3]; int *y=x; y[3]=3; return x[1][0]; }"
assert 4 "int main() { int x[2][3]; int *y=x; y[4]=4; return x[1][1]; }"
assert 5 "int main() { int x[2][3]; int *y=x; y[5]=5; return x[1][2]; }"
assert 6 "int main() { int x[2][3]; int *y=x; y[6]=6; return x[2][0]; }"

# sizeof 演算子に対応してテストケース
assert 8 "int main() { int x; return sizeof(x); }"
assert 8 "int main() { int x; return sizeof x; }"
assert 8 "int main() { int *x; return sizeof(x); }"
assert 32 "int main() { int x[4]; return sizeof(x); }"
assert 96 "int main() { int x[3][4]; return sizeof(x); }"
assert 32 "int main() { int x[3][4]; return sizeof(*x); }"
assert 8 "int main() { int x[3][4]; return sizeof(**x); }"
assert 9 "int main() { int x[3][4]; return sizeof(**x) + 1; }"
assert 9 "int main() { int x[3][4]; return sizeof **x + 1; }"
assert 8 "int main() { int x[3][4]; return sizeof(**x + 1); }"

# global 変数に対応したテストケース
assert 0 "int x; int main() { return x; }"
assert 3 "int x; int main() { x=3; return x; }"
assert 0 "int x[4]; int main() { x[0]=0; x[1]=1; x[2]=2; x[3]=3; return x[0]; }"
assert 1 "int x[4]; int main() { x[0]=0; x[1]=1; x[2]=2; x[3]=3; return x[1]; }"
assert 2 "int x[4]; int main() { x[0]=0; x[1]=1; x[2]=2; x[3]=3; return x[2]; }"
assert 3 "int x[4]; int main() { x[0]=0; x[1]=1; x[2]=2; x[3]=3; return x[3]; }"

assert 8 "int x; int main() { return sizeof(x); }"
assert 32 "int x[4]; int main() { return sizeof(x); }"

# char 型に対応したテストケース
assert 1 "int main() { char x=1; return x; }"
assert 1 "int main() { char x=1; char y=2; return x; }"
assert 2 "int main() { char x=1; char y=2; return y; }"

assert 1 "int main() { char x; return sizeof(x); }"
assert 10 "int main() { char x[10]; return sizeof(x); }"
assert 1 "int main() { return sub_char(7, 3, 3); } int sub_char(char a, char b, char c) { return a-b-c; }"

# 文字列リテラルに対応したテストケース
assert 97 'int main() { return "abc"[0]; }'
assert 98 'int main() { return "abc"[1]; }'
assert 99 'int main() { return "abc"[2]; }'
assert 0 'int main() { return "abc"[3]; }'
assert 4 'int main() { return sizeof("abc"); }'

# エスケープ文字に対応したテストケース
assert 7 'int main() { return "\a"[0]; }'
assert 8 'int main() { return "\b"[0]; }'
assert 9 'int main() { return "\t"[0]; }'
assert 10 'int main() { return "\n"[0]; }'
assert 11 'int main() { return "\v"[0]; }'
assert 12 'int main() { return "\f"[0]; }'
assert 13 'int main() { return "\r"[0]; }'
assert 27 'int main() { return "\e"[0]; }'
assert 0 'int main() { return "\0"[0]; }'

assert 106 'int main() { return "\j"[0]; }'
assert 107 'int main() { return "\k"[0]; }'
assert 108 'int main() { return "\l"[0]; }'

echo OK
