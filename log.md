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

# その他
- まず、テストケースを実装すると全体像が把握しやすいかも。
- 方針がなんとなく立てば、とりあえず実装してみて、前半の取っていないといけないテストが通らない時は、前回の実装との整合性が取れておらずバグが生じているので、その調査に取りかかる。
