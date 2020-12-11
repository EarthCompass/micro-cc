#pragma once
#include <iostream>
#include <llvm/IR/Value.h>
#include <vector>

#define PRINTTAB                                                               \
  for (size_t _iiii = 0; _iiii < level; _iiii++) {                                     \
    std::cout << "  ";                                                     \
  }

namespace microcc {
class CodeGenContext;
class Node {
public:
  virtual llvm::Value *codeGen(CodeGenContext &context) {
    return (llvm::Value *)0;
  }
  virtual void PrintAST(int level) {}
};
class Stmt :public Node{
public:
  // virtual void PrintAST(int level) {}
};
class Expr : public Node {
public:
  // virtual void PrintAST(int level) {}
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
    // std::cout<<level<<"\n";
    PRINTTAB
    std::cout << "IntegerLiteralExpr :" << value << "\n";
  }
};
class DoubleLiteralExpr : public Expr {
public:
  double value;
  DoubleLiteralExpr(double value) : value(value) {}
  virtual void PrintAST(int level) {
    // std::cout<<level<<"\n";
    PRINTTAB
    std::cout << "DoubleLiteralExpr :" << value << "\n";
  }
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
    // std::cout<<level<<"\n";
    PRINTTAB
    std::cout << "BinaryOperatorExpr :" << op << "\n";
    lhs.get()->PrintAST(level + 1);
    rhs.get()->PrintAST(level + 1);
  }
};


class VarDeclStmt : public Stmt {
public:
  std::unique_ptr<IndentifierExpr> type;
  std::unique_ptr<IndentifierExpr> id;
  std::unique_ptr<Expr> expr;
  VarDeclStmt(std::unique_ptr<IndentifierExpr> type,
              std::unique_ptr<IndentifierExpr> id,
              std::unique_ptr<Expr> expr)
      : type(std::move(type)), id(std::move(id)),expr(std::move(expr)) {}
  virtual void PrintAST(int level) {
    PRINTTAB
    std::cout << "VarDeclStmt"
              << "\n";
    type->PrintAST(level + 1);
    id->PrintAST(level + 1);
    if(expr)
      expr->PrintAST(level+1);
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
    std::cout<<"SingleExprStmt\n";
    expr->PrintAST(level + 1);
  }
};
class CompoundStmt : public Stmt {
public:
  std::unique_ptr<Stmts>stmts;
  CompoundStmt(std::unique_ptr<Stmts> stmts)
      : stmts(std::move(stmts)) {}
  virtual void PrintAST(int level){
    PRINTTAB
    std::cout << "CompoundStmt"
              << "\n";
    stmts->PrintAST(level+1);
  }
};
class ReturnStmt : public Stmt{
  public:
  std::unique_ptr<Expr> expr;
  ReturnStmt(std::unique_ptr<Expr> expr):expr(std::move(expr)){};
  virtual void PrintAST(int level){
    PRINTTAB
    std::cout<<"ReturnStmt"<<"\n";
    expr->PrintAST(level+1);
  }
};

class VarDeclExpr {
  public:
  std::unique_ptr<IndentifierExpr> type;
  std::unique_ptr<IndentifierExpr> id;
  VarDeclExpr(std::unique_ptr<IndentifierExpr>type,std::unique_ptr<IndentifierExpr>id):
  type(std::move(type)),id(std::move(id)){};
};

typedef std::vector<std::unique_ptr<VarDeclExpr>> FuncDecArgsList;

class FuncDeclStmt : public Stmt {
public:
  std::unique_ptr<IndentifierExpr> type;
  std::unique_ptr<IndentifierExpr> id;
  std::unique_ptr<FuncDecArgsList> args;
  std::unique_ptr<CompoundStmt> funcBody;
  FuncDeclStmt(std::unique_ptr<IndentifierExpr> type,
        std::unique_ptr<IndentifierExpr> id,
        std::unique_ptr<FuncDecArgsList> args,
        std::unique_ptr<CompoundStmt> funcBody):
        type(std::move(type)),id(std::move(id)),args(std::move(args)),funcBody(std::move(funcBody)){};

  virtual void PrintAST(int level){
      PRINTTAB
      std::cout<<"FuncDeclStmt "<<"\n";
      for(auto arg = args->begin();arg!=args->end();arg++){
          PRINTTAB std::cout<<"arg type: ";std::cout<<arg->get()->type->name<<" ";
          PRINTTAB std::cout<<"arg name: ";std::cout<<arg->get()->id->name<<std::endl;
      }
      PRINTTAB std::cout<<"return type: ";std::cout<<type->name<<std::endl;
      PRINTTAB std::cout<<"function name: ";std::cout<<id->name<<std::endl;
      PRINTTAB std::cout<<"function body: \n";funcBody->PrintAST(level+1);
  }
};

typedef std::vector<std::unique_ptr<Expr>> CallArgs;

class CallExpr : public Expr {
public:
  std::unique_ptr<IndentifierExpr> callee;
  std::unique_ptr<CallArgs> args;
  CallExpr(std::unique_ptr<IndentifierExpr> callee,
           std::unique_ptr<CallArgs> args)
      : callee(std::move(callee)), args(std::move(args)) {};
    virtual void PrintAST(int level){
      PRINTTAB
      std::cout<<"CallExpr :"<<"\n";
      callee->PrintAST(level+1);
      for(auto arg = args->begin();arg!=args->end();arg++){
        arg->get()->PrintAST(level+1);
      }
  }
};

class IfStmt : public Stmt {};
} // namespace microcc
