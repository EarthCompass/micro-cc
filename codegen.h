#include <llvm/IR/Value.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LegacyPassManager.h>
#include "Nodes.hpp"
#include "parser.h"

using namespace llvm;
using namespace std;


namespace microcc {
    class CodeContext {
    public:
        LLVMContext context;
        unique_ptr<Module> theModule;
        IRBuilder<> builder;
        std::map<std::string, Value *> localSymbol;
        void IRGen(Stmts&root){
            cout<<"Generating IR code in context"<<endl;
            Value * p = root.codeGen(*this);
            p->print(errs());
        }
        CodeContext():builder(context){
            std::make_unique<Module>("test", context);
        }
    };

    std::unique_ptr<Expr> LogError(const char *Str) {
        fprintf(stderr, "Error: %s\n", Str);
        return nullptr;
    }

    Value *LogErrorV(const string &str) {
        fprintf(stderr, "Error: %s\n", str.c_str());
        return nullptr;
    }

    Value *IntegerLiteralExpr::codeGen(CodeContext &context) {
        cout << "Gen IntegerLiteral:" << value << endl;
        // return (llvm::Value*)0;
        return ConstantInt::get(Type::getInt32Ty(context.context), value, true);
    }

    Value *DoubleLiteralExpr::codeGen(CodeContext &context) {
        cout << "Gen IntegerLiteral:" << value << endl;
        return ConstantFP::get(Type::getDoubleTy(context.context), value);
    }

    Value *IdentifierExpr::codeGen(CodeContext &context) {
        cout << "Gen IdentifierRef:" << name << endl;
        Value *V = context.localSymbol[name];
        if (!V)
            return LogErrorV("undefined variable\n");
        return V;
    }

    Value *BinaryOperatorExpr::codeGen(CodeContext &context) {
        Value *L = lhs->codeGen(context);
        Value *R = rhs->codeGen(context);
        if (op != T_ASSIGN) {
            bool hasDouble = false;
            if (L->getType()->getTypeID() == Type::DoubleTyID || R->getType()->getTypeID() == Type::DoubleTyID) {
                if (L->getType()->getTypeID() == Type::DoubleTyID) {
                    R = context.builder.CreateSIToFP(R, Type::getDoubleTy(context.context), "si2dt");
                }
                if (R->getType()->getTypeID() == Type::DoubleTyID) {
                    L = context.builder.CreateSIToFP(R, Type::getDoubleTy(context.context), "si2dt");
                }
                hasDouble = true;
            }
            switch (op) {
                case T_ADD:
                    return hasDouble ? context.builder.CreateFAdd(L, R, "adddt") : context.builder.CreateAdd(L, R,
                                                                                                             "addst");
                case T_MINUS:
                    return hasDouble ? context.builder.CreateFSub(L, R, "subdt") : context.builder.CreateSub(L, R,
                                                                                                             "subst");
                case T_MUL:
                    return hasDouble ? context.builder.CreateFMul(L, R, "muldt") : context.builder.CreateMul(L, R,
                                                                                                             "mulst");
                case T_DIV:
                    return hasDouble ? context.builder.CreateFDiv(L, R, "divdt") : context.builder.CreateSDiv(L, R,
                                                                                                              "divst");
                case T_MOD:
                    return hasDouble ? context.builder.CreateFRem(L, R, "moddt") : context.builder.CreateSRem(L, R,
                                                                                                              "modst");
                case T_GT:
                    return hasDouble ? context.builder.CreateFCmpOGT(L, R, "cmpgtdt") : context.builder.CreateICmpSGT(L,
                                                                                                                      R,
                                                                                                                      "cmpgtst");
                case T_GE:
                    return hasDouble ? context.builder.CreateFCmpOGE(L, R, "cmpgedt") : context.builder.CreateICmpSGE(L,
                                                                                                                      R,
                                                                                                                      "cmpgest");
                case T_LT:
                    return hasDouble ? context.builder.CreateFCmpOLT(L, R, "cmpltdt") : context.builder.CreateICmpSLT(L,
                                                                                                                      R,
                                                                                                                      "cmpltst");
                case T_LE:
                    return hasDouble ? context.builder.CreateFCmpOLE(L, R, "cmpledt") : context.builder.CreateICmpSLE(L,
                                                                                                                      R,
                                                                                                                      "cmplest");
                case T_EQUAL:
                    return hasDouble ? context.builder.CreateFCmpOEQ(L, R, "cmpeqdt") : context.builder.CreateICmpEQ(L,
                                                                                                                     R,
                                                                                                                     "cmpeqst");
                default:
                    return LogErrorV("unknown...\n");
            }


        } else {
            return LogErrorV("to do...\n");
        }
    }

    Value *SingleExprStmt::codeGen(CodeContext &context) {
        cout << "Gen SingleExprStmt" << endl;
        return expr->codeGen(context);
    }

    Value *Stmts::codeGen(CodeContext &context) {
        cout << "Gen Stmts" << endl;
        Value * p = nullptr;
        for (auto &stmt:stmts) {
            if (stmt)
                p = stmt->codeGen(context);
        }
        return p;
    }
}
