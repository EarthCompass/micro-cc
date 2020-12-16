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
        std::map<std::string, AllocaInst *> localSymbol;

        void IRGen(Stmts &root) {
            cout << "Generating IR code in context" << endl;
            std::vector<Type*> sysArgs;
            FunctionType* mainFuncType = FunctionType::get(Type::getVoidTy(this->context), makeArrayRef(sysArgs), false);
            Function* mainFunc = Function::Create(mainFuncType, GlobalValue::ExternalLinkage, "main",theModule.get());
            BasicBlock* block = BasicBlock::Create(this->context, "entry",mainFunc);
            builder.SetInsertPoint(block);
            Value *p = root.codeGen(*this);
            cout << "IR code:" << endl;
            theModule->print(outs(), nullptr);
        }

        CodeContext() : builder(context) {
            theModule = std::make_unique<Module>("test", context);
        }

        Type *getType(const string& target) {
            if (!target.empty()) {
                if (target == "int")
                    return Type::getInt32Ty(context);
                else if (target == "double")
                    return Type::getDoubleTy(context);
                else
                    return nullptr;
            } else
                return nullptr;
        }
    };

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
        if (!isType) {
            cout << "Gen IdentifierRef:" << name << endl;
            if (Value *V = context.localSymbol[name])
                return context.builder.CreateLoad(V);
            else if(auto G = context.theModule->getGlobalVariable(name))
                return context.builder.CreateLoad(G);
            else
                return LogErrorV("undefined variable " + name + "\n");
        }
        else
            return nullptr;
    }

    Value *BinaryOperatorExpr::codeGen(CodeContext &context) {
        cout << "Gen BinaryOperatorExpr" << endl;
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

    Value *VarDeclStmt::codeGen(CodeContext &context) {
        cout << "Gen VarDeclStmt " << "Type:" << type->name << " Name:" << id->name << endl;
        AllocaInst *p = nullptr;
        if(isRoot){
            GlobalVariable* G=nullptr;
            if (type->name == "int") {
                context.theModule->getOrInsertGlobal(id->name,Type::getInt32Ty(context.context));
                G = context.theModule->getGlobalVariable(id->name);
                G->setInitializer(ConstantInt::get(Type::getInt32Ty(context.context),0,true));
            } else if (type->name == "double") {
                context.theModule->getOrInsertGlobal(id->name,Type::getDoubleTy(context.context));
                G = context.theModule->getGlobalVariable(id->name);
                G->setInitializer(ConstantInt::get(Type::getDoubleTy(context.context),0,true));
            }
        }else{
            if (type->name == "int") {
                p = context.builder.CreateAlloca(Type::getInt32Ty(context.context));
            } else if (type->name == "double") {
                p = context.builder.CreateAlloca(Type::getDoubleTy(context.context));
            }
            context.localSymbol[id->name] = p;
        }
//        if (expr){
//            Value * q = expr->codeGen(context);
//            context.builder.Creas
//        }
        cout << "end" << endl;
        return p;
//        Value*p=context.builder.CreateAlloca()
//        context.builder.CreateVa
    }

    Value *SingleExprStmt::codeGen(CodeContext &context) {
        cout << "Gen SingleExprStmt" << endl;
        return expr->codeGen(context);
    }

    Value *Stmts::codeGen(CodeContext &context) {
        cout << "Gen Stmts" << endl;
        Value *p = nullptr;
        for (auto &stmt:stmts) {
            if (stmt)
                p = stmt->codeGen(context);
        }
        return p;
    }

    Value *FuncDeclStmt::codeGen(CodeContext &context) {
        cout << "Gen FuncDeclStmt:" << endl;
        cout << "Function return type:" << type->name << endl;
        cout << "Function name:" << id->name << endl;
        cout << "Function args:" << endl;

        std::vector<Type*> argTypes;
        for (auto &arg:*args) {
            cout << "Type: " << arg->type->name << ",Name: " << arg->id->name << endl;
            argTypes.push_back(context.getType(arg->type->name));
        }
        FunctionType * funcType = FunctionType::get(context.getType(type->name),argTypes,false);
        Function * func = Function::Create(funcType,GlobalValue::ExternalLinkage,id->name,context.theModule.get());
        return func;
    }
}
