#include <iostream>
#include <llvm/IR/Value.h>
#include <vector>
#define PRINTTAB                                                               \
  for (size_t i = 0; i < level; i++) {                                         \
    std::cout << "    ";                                                       \
  }

namespace microcc {
class CodeGenContext;
class Node {
public:
  ~Node() {}
  virtual llvm::Value *codeGen(CodeGenContext &context) {
    return (llvm::Value *)0;
  }
  virtual void PrintAST(int level) {}
};
class Stmt {
public:
  virtual void PrintAST(int level) {}
};
class Decl {};
class Expr {
public:
  //   virtual ~Expr();
  virtual void PrintAST(int level) {}
};
class IndentifierExpr : public Expr {
public:
  std::string name;
  bool isType;
  IndentifierExpr(std::string *name, bool isType)
      : name(*name), isType(isType) {}
  virtual void PrintAST(int level) {
    PRINTTAB
    std::cout << "IndentifierExpr ";
    if (isType)
      std::cout << "Type";
    std::cout << ": " << name << "\n";
  }
};
class IntegerLiteralExpr : public Expr {
public:
  int value;
  IntegerLiteralExpr(int value) : value(value) {}
  virtual void PrintAST(int level) {
    PRINTTAB
    std::cout << "IntegerLiteralExpr :" << value << "\n";
  }
};
class DoubleLiteralExpr : public Expr {
public:
  double value;
  DoubleLiteralExpr(double value) : value(value) {}
};
class BinaryOperatorExpr : public Expr {
public:
  int op;
  std::unique_ptr<Expr> lhs;
  std::unique_ptr<Expr> rhs;
  BinaryOperatorExpr(int op, std::unique_ptr<Expr> lhs,
                     std::unique_ptr<Expr> rhs)
      : op(op), lhs(std::move(lhs)), rhs(std::move(rhs)) {}
  virtual void PrintAST(int level) {
    PRINTTAB
    std::cout << "BinaryOperatorExpr :" << op << "\n";
    lhs.get()->PrintAST(level + 1);
    rhs.get()->PrintAST(level + 1);
  }
};
class CallExpr : public Expr {
public:
  std::unique_ptr<IndentifierExpr> callee;
  std::vector<std::unique_ptr<Expr>> args;
  CallExpr(std::unique_ptr<IndentifierExpr> &callee,
           std::vector<std::unique_ptr<Expr>> args)
      : callee(std::move(callee)), args(std::move(args)) {}
};
class CoumpondStmt : public Stmt {
public:
  std::vector<std::unique_ptr<Stmt>> stmts;
  CoumpondStmt(std::vector<std::unique_ptr<Stmt>> &stmts)
      : stmts(std::move(stmts)) {}
};
class FuncDeclStmt : public Stmt {
public:
  std::unique_ptr<IndentifierExpr> type;
  std::unique_ptr<IndentifierExpr> id;
  std::vector<std::unique_ptr<Expr>> args;
  std::unique_ptr<CoumpondStmt> funcBody;
};
class VarDeclStmt : public Stmt {
public:
  std::unique_ptr<IndentifierExpr> type;
  std::unique_ptr<IndentifierExpr> id;
  VarDeclStmt(std::unique_ptr<IndentifierExpr> type,
              std::unique_ptr<IndentifierExpr> id)
      : type(std::move(type)), id(std::move(id)) {}
  virtual void PrintAST(int level) {
    PRINTTAB
    std::cout << "VarDeclStmt"
              << "\n";
    type->PrintAST(level + 1);
    id->PrintAST(level + 1);
  }
};
class Stmts : public Stmt {
public:
  std::vector<std::unique_ptr<Stmt>> stmts;
  virtual void PrintAST(int level) {
    PRINTTAB
    std::cout << "Stmts\n";
    for (auto i = stmts.begin(); i != stmts.end(); i++) {
      i->get()->PrintAST(level + 1);
    }
  }
  // Stmts(std::vector<std::unique_ptr<Stmt>> stmts):stmts(std::move(stmts)){}
};
class SingleExprStmt : public Stmt {
public:
  std::unique_ptr<Expr> expr;
  SingleExprStmt(std::unique_ptr<Expr> expr) : expr(std::move(expr)){};
  virtual void PrintAST(int level) {
    PRINTTAB
    std::cout << "SingleExprStmt"
              << "\n";
    expr->PrintAST(level + 1);
  }
};
class IfStmt : public Stmt {};
class VarDecl : public Decl {};
class FuncDecl : public Decl {};
} // namespace microcc
