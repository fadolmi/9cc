#include "9cc.h"

int main(int argc, char* argv[]) {
  if (argc != 2) {
	error("引数の個数が正しくありません");
	return 1;
  }

  // トークナイズしてパースする
  user_input = argv[1];
  token = tokenize();
  program();

  // アセンブリを出力
  gen_init();

  prologue();

  for (int i = 0; code[i]; i++) {
	gen(code[i]);

	// 式の評価結果としてスタックに一つの値が残っている
    // はずなので、スタックが溢れないようにポップしておく
    printf("  pop rax\n");
  }

  epilogue();

  return 0;
}
