#include <iostream>
#include <llvm/IR/Value.h>

namespace microcc {
class CodeGenContext;
class Node {
public:
  ~Node() {}
  virtual llvm::Value *codeGen(CodeGenContext &context) {
    return (llvm::Value *)0;
  }
};
class Stmt {};
class Decl {};
class Expr {
public:
  virtual ~Expr();
};
class IntegerLiteralExpr : public Expr {
public:
  int Value;
  IntegerLiteralExpr(int Value) : Value(Value) {}
};
class DoubleLiteralExpr : public Expr {
public:
  double Value;
  DoubleLiteralExpr(double Value) : Value(Value) {}
};
class BinaryOperatorExpr : public Expr {
public:
  int Op;
  std::unique_ptr<Expr> Lhs;
  std::unique_ptr<Expr> Rhs;
  BinaryOperatorExpr(int Op, std::unique_ptr<Expr> Lhs,
                     std::unique_ptr<Expr> Rhs)
      : Op(Op), Lhs(std::move(Lhs)), Rhs(std::move(Rhs)) {}
};
class IfStmt : public Stmt {};
class VarDecl : public Decl {};
class FuncDecl : public Decl {};
} // namespace microcc
