#pragma once

#include <iostream>
#include <llvm/IR/Value.h>
#include <vector>

#define PRINTTAB                                                               \
  for (size_t _iiii = 0; _iiii < level; _iiii++) {                                     \
    std::cout << "  ";                                                     \
  }

namespace microcc {
    class CodeContext;

    class Node {
    public:
        virtual llvm::Value *codeGen(CodeContext &context){return nullptr;}

        virtual void PrintAST(int level) {}
    };

    class Stmt : public Node {
    public:
        // virtual void PrintAST(int level) {}
    };

    class Expr : public Node {
    public:
    };

    class IdentifierExpr : public Expr {
    public:
        std::string name;
        bool isType;

        IdentifierExpr(std::string *name, bool isType)
                : name(*name), isType(isType) {}

        void PrintAST(int level) override {
            PRINTTAB
            std::cout << "IdentifierExpr ";
            if (isType)
                std::cout << "Type";
            std::cout << ": " << name << "\n";
        }

        llvm::Value *codeGen(CodeContext &context) override;
    };

    class IntegerLiteralExpr : public Expr {
    public:
        int value;

        explicit IntegerLiteralExpr(int value) : value(value) {}

        void PrintAST(int level) override {
            PRINTTAB
            std::cout << "IntegerLiteralExpr :" << value << "\n";
        }

        llvm::Value *codeGen(CodeContext &context) override;
    };

    class DoubleLiteralExpr : public Expr {
    public:
        double value;

        explicit DoubleLiteralExpr(double value) : value(value) {}

        void PrintAST(int level) override {
            PRINTTAB
            std::cout << "DoubleLiteralExpr :" << value << "\n";
        }

        llvm::Value *codeGen(CodeContext &context) override;
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
            lhs->PrintAST(level + 1);
            rhs->PrintAST(level + 1);
        }

        llvm::Value *codeGen(CodeContext &context) override;
    };


    class VarDeclStmt : public Stmt {
    public:
        std::unique_ptr<IdentifierExpr> type;
        std::unique_ptr<IdentifierExpr> id;
        std::unique_ptr<Expr> expr;

        VarDeclStmt(std::unique_ptr<IdentifierExpr> type,
                    std::unique_ptr<IdentifierExpr> id,
                    std::unique_ptr<Expr> expr)
                : type(std::move(type)), id(std::move(id)), expr(std::move(expr)) {}

        void PrintAST(int level) override {
            PRINTTAB
            std::cout << "VarDeclStmt"
                      << "\n";
            type->PrintAST(level + 1);
            id->PrintAST(level + 1);
            if (expr)
                expr->PrintAST(level + 1);
        }
    };

    class Stmts : public Stmt {
    public:
        std::vector<std::unique_ptr<Stmt>> stmts;

        void PrintAST(int level) override {
            PRINTTAB
            std::cout << "Stmts\n";
            for (auto &stmt : stmts) {
                stmt->PrintAST(level + 1);
            }
        }
        llvm::Value *codeGen(CodeContext &context) override;
        // Stmts(std::vector<std::unique_ptr<Stmt>> stmts):stmts(std::move(stmts)){}
    };

    class SingleExprStmt : public Stmt {
    public:
        std::unique_ptr<Expr> expr;

        explicit SingleExprStmt(std::unique_ptr<Expr> expr) : expr(std::move(expr)) {};

        void PrintAST(int level) override {
            PRINTTAB
            std::cout << "SingleExprStmt\n";
            expr->PrintAST(level + 1);
        }
        llvm::Value *codeGen(CodeContext &context) override;
    };

    class CompoundStmt : public Stmt {
    public:
        std::unique_ptr<Stmts> stmts;

        explicit CompoundStmt(std::unique_ptr<Stmts> stmts)
                : stmts(std::move(stmts)) {}

        void PrintAST(int level) override {
            PRINTTAB
            std::cout << "CompoundStmt"
                      << "\n";
            stmts->PrintAST(level + 1);
        }
    };

    class ReturnStmt : public Stmt {
    public:
        std::unique_ptr<Expr> expr;

        explicit ReturnStmt(std::unique_ptr<Expr> expr) : expr(std::move(expr)) {};

        void PrintAST(int level) override {
            PRINTTAB
            std::cout << "ReturnStmt" << "\n";
            expr->PrintAST(level + 1);
        }
    };

    class VarDeclExpr {
    public:
        std::unique_ptr<IdentifierExpr> type;
        std::unique_ptr<IdentifierExpr> id;

        VarDeclExpr(std::unique_ptr<IdentifierExpr> type, std::unique_ptr<IdentifierExpr> id) :
                type(std::move(type)), id(std::move(id)) {};
    };

    typedef std::vector<std::unique_ptr<VarDeclExpr>> FuncDecArgsList;

    class FuncDeclStmt : public Stmt {
    public:
        std::unique_ptr<IdentifierExpr> type;
        std::unique_ptr<IdentifierExpr> id;
        std::unique_ptr<FuncDecArgsList> args;
        std::unique_ptr<CompoundStmt> funcBody;

        FuncDeclStmt(std::unique_ptr<IdentifierExpr> type,
                     std::unique_ptr<IdentifierExpr> id,
                     std::unique_ptr<FuncDecArgsList> args,
                     std::unique_ptr<CompoundStmt> funcBody) :
                type(std::move(type)), id(std::move(id)), args(std::move(args)), funcBody(std::move(funcBody)) {};

        void PrintAST(int level) override {
            PRINTTAB
            std::cout << "FuncDeclStmt " << "\n";
            for (auto &arg : *args) {
                PRINTTAB
                std::cout << "arg type: ";
                std::cout << arg->type->name << " ";
                PRINTTAB
                std::cout << "arg name: ";
                std::cout << arg.get()->id->name << std::endl;
            }
            PRINTTAB
            std::cout << "return type: ";
            std::cout << type->name << std::endl;
            PRINTTAB
            std::cout << "function name: ";
            std::cout << id->name << std::endl;
            PRINTTAB
            std::cout << "function body: \n";
            funcBody->PrintAST(level + 1);
        }
    };

    typedef std::vector<std::unique_ptr<Expr>> CallArgs;

    class CallExpr : public Expr {
    public:
        std::unique_ptr<IdentifierExpr> callee;
        std::unique_ptr<CallArgs> args;

        CallExpr(std::unique_ptr<IdentifierExpr> callee,
                 std::unique_ptr<CallArgs> args)
                : callee(std::move(callee)), args(std::move(args)) {};

        void PrintAST(int level) override {
            PRINTTAB
            std::cout << "CallExpr :" << "\n";
            callee->PrintAST(level + 1);
            for (auto &arg : *args) {
                arg->PrintAST(level + 1);
            }
        }
    };

    class IfStmt : public Stmt {
    };
} // namespace microcc
