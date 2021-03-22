#include "9cc.h"

/* 
  生成規則
	program    = stmt*
	stmt       = expr ";"
	expr       = assign
	assign     = equality ("=" assign)?
	equality   = relational ("==" relational | "!=" relational)*
	relational = add ("<" add | "<=" add | ">" add | ">=" add)*
	add        = unary ("*" unary | "/" unary);
	unary      = (("+" | "-") unary) | primary
	primary    = num | ident | "(" expr ")"
*/

Node* new_node(NodeKind kind, Node* lhs, Node* rhs) {
  Node* node = calloc(1, sizeof(Node));
  node->kind = kind;
  node->lhs = lhs;
  node->rhs = rhs;
  return node;
}

Node* new_node_num(int val) {
  Node* node = calloc(1, sizeof(Node));
  node->kind = ND_NUM;
  node->val = val;
  return node;
}

Node* new_node_ident(int offset) {
  Node* node = calloc(1, sizeof(Node));
  node->kind = ND_LVAR;
  node->offset = offset;
  return node;
}

// プロトタイプ宣言
Node* stmt();
Node* expr();
Node* assign();
Node* equality();
Node* relational();
Node* add();
Node* mul();
Node* unary();
Node* primary();

// 複数のノードを保存する
Node* code[100];

// program = stmt*
void program() {
  int i = 0;
  while (!at_eof()) {
	code[i++] = stmt();
  }
  code[i] = NULL;
}

// stmt = expr ";"
Node* stmt() {
  Node* node = expr();
  expect(";");
  return node;
}

// expr = assign
Node* expr() {
  return assign();
}

// assign = equality ("=" assign)?
Node* assign() {
  Node* node = equality();
  if (consume("=")){
	node = new_node(ND_ASSIGN, node, assign());
  }
  return node;
}

// equality = relational ("==" relational | "!=" relational)*
Node* equality() {
  Node* node = relational();

  for(;;) {
	if (consume("==")) {
	  node = new_node(ND_EQ, node, mul());
	}else if (consume("!=")) {
	  node = new_node(ND_NE, node, mul()); 
	}else {
	  return node;
	}
  }
}

// relational = add ("<" add | "<=" add | ">" add | ">=" add)*
Node* relational() {
  Node* node = add();

  for(;;) {
	if (consume("<")) {
	  node = new_node(ND_LT, node, add());
	}else if (consume("<=")) {
	  node = new_node(ND_LE, node, add()); 
	}else if (consume(">")) { 
	  node = new_node(ND_LT, add(), node); 
	}else if (consume(">=")) {
	  node = new_node(ND_LE, add(), node);
	}else {
	  return node;
	}
  }
}

// add = mul ("+" mul | "-" mul)*
Node* add() {
  Node* node = mul();

  for(;;) {
	if (consume("+")) {
	  node = new_node(ND_ADD, node, mul());
	}else if (consume("-")) {
	  node = new_node(ND_SUB, node, mul()); 
	}else {
	  return node;
	}
  }
}

// mul = unary ("*" unary | "/" unary)*
Node* mul() {
  Node* node = unary();

  for(;;) {
	if (consume("*")) {
	  node = new_node(ND_MUL, node, unary());
	}else if (consume("/")) {
	  node = new_node(ND_DIV, node, unary());
	}else {
	  return node;
	}
  }
}

// unary   = (("+" | "-") unary) | primary
Node* unary() {
  if (consume("+")) {
	return unary();
  }else if (consume("-")) {
	return new_node(ND_SUB, new_node_num(0), unary());
  }else {
	return primary();
  }
}

// primary = num | ident | "(" expr ")"
Node* primary() {
  // 次のトークンが"("なら"(" expr ")"のはず
  if (consume("(")) {
	Node* node = expr();
	expect(")");
	return node;
  }else if (token->kind == TK_IDENT) {
	// ローカル変数ならベースポインタからのオフセットを求める
	int offset = (token->str[0] - 'a' + 1) * 8;
	token = token->next;
	return new_node_ident(offset);
  }else {
	// そうでなければ数値のはず
	return new_node_num(expect_number());
  }
}
