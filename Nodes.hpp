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
        bool isRoot = false;
        int line=-1;
        int col=-1;
        virtual void PrintAST(int level) {}
        virtual llvm::Value *codeGen(CodeContext &context) { return nullptr; }

    };

    class Stmt : public Node {
    public:

    };

    class Expr : public Node {
    public:
        bool isMutable=false;
        bool isAssign=false;

    };

    class IdentifierExpr : public Expr {
    public:
        std::string name;
        bool isType;
        bool isRef = true;

        IdentifierExpr(std::string *name, bool isType, int line1, int col1)
                : name(*name), isType(isType) {
            line = line1;
            col = col1;
        }

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

        explicit IntegerLiteralExpr(int value, int line1, int col1) : value(value) {
            line = line1;
            col = col1;
        }

        void PrintAST(int level) override {
            PRINTTAB
            std::cout << "IntegerLiteralExpr :" << value << "\n";
        }

        llvm::Value *codeGen(CodeContext &context) override;
    };

    class DoubleLiteralExpr : public Expr {
    public:
        double value;

        explicit DoubleLiteralExpr(double value, int line1, int col1) : value(value) {
            line = line1;
            col = col1;
        }

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
                           std::unique_ptr<Expr> rhs,
                           int line1, int col1)
                : op(op), lhs(std::move(lhs)), rhs(std::move(rhs)) {
            line = line1;
            col = col1;
        }

        void PrintAST(int level) override {
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
                    std::unique_ptr<Expr> expr,
                    int line1, int col1)
                : type(std::move(type)), id(std::move(id)), expr(std::move(expr)) {
            line = line1;
            col = col1;
            this->id->isRef = false;
        }

        void PrintAST(int level) override {
            PRINTTAB
            std::cout << "VarDeclStmt"
                      << "\n";
            type->PrintAST(level + 1);
            id->PrintAST(level + 1);
            if (expr)
                expr->PrintAST(level + 1);
        }

        llvm::Value *codeGen(CodeContext &context) override;
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
    };

    class SingleExprStmt : public Stmt {
    public:
        std::unique_ptr<Expr> expr;

        explicit SingleExprStmt(std::unique_ptr<Expr> expr, int line1, int col1) : expr(std::move(expr)) {
            line = line1;
            col = col1;
        };

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

        explicit CompoundStmt(std::unique_ptr<Stmts> stmts,
                              int line1, int col1)
                : stmts(std::move(stmts)) {
            line = line1;
            col = col1;
        }

        void PrintAST(int level) override {
            PRINTTAB
            std::cout << "CompoundStmt"
                      << "\n";
            if (stmts)
                stmts->PrintAST(level + 1);

        }
        llvm::Value *codeGen(CodeContext &context) override;

    };

    class ReturnStmt : public Stmt {
    public:
        std::unique_ptr<Expr> expr;

        explicit ReturnStmt(std::unique_ptr<Expr> expr, int line1, int col1) : expr(std::move(expr)) {
            line = line1;
            col = col1;
        };

        void PrintAST(int level) override {
            PRINTTAB
            std::cout << "ReturnStmt" << "\n";
            expr->PrintAST(level + 1);
        }
        llvm::Value *codeGen(CodeContext &context) override;
    };

    class VarDeclExpr : public Expr {
    public:
        std::unique_ptr<IdentifierExpr> type;
        std::unique_ptr<IdentifierExpr> id;

        VarDeclExpr(std::unique_ptr<IdentifierExpr> type, std::unique_ptr<IdentifierExpr> id, int line1, int col1) :
                type(std::move(type)), id(std::move(id)) {
            line = line1;
            col = col1;
            this->id->isRef = false;
        };
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
                     std::unique_ptr<CompoundStmt> funcBody,
                     int line1, int col1) :
                type(std::move(type)), id(std::move(id)), args(std::move(args)), funcBody(std::move(funcBody)) {
            line = line1;
            col = col1;
        };

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

        llvm::Value *codeGen(CodeContext &context) override;
    };

    typedef std::vector<std::unique_ptr<Expr>> CallArgs;

    class CallExpr : public Expr {
    public:
        std::unique_ptr<IdentifierExpr> callee;
        std::unique_ptr<CallArgs> args;

        CallExpr(std::unique_ptr<IdentifierExpr> callee,
                 std::unique_ptr<CallArgs> args,
                 int line1, int col1)
                : callee(std::move(callee)), args(std::move(args)) {
            line = line1;
            col = col1;
        };

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
