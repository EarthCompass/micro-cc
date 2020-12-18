#include <stack>
#include <llvm/IR/Value.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/PassManager.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Target/TargetOptions.h>
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
        std::stack<BasicBlock *> bbs;

        void IRGen(Stmts &root) {
            cout << "Generating IR code in context" << endl;
            std::vector<Type *> sysArgs;
            BasicBlock *block = BasicBlock::Create(this->context, "entry");
            this->pushBasicBlock(block);
            Value *p = root.codeGen(*this);
            cout << "IR code:" << endl;
            if(!theModule->getFunction("main")){
                cerr<<"\"main\" function not found"<<endl;
            }
            theModule->print(outs(), nullptr);
        }
        void ObjectGen(){
            InitializeAllTargetInfos();
            InitializeAllTargets();
            InitializeAllTargetMCs();
            InitializeAllAsmParsers();
            InitializeAllAsmPrinters();
        }
        CodeContext() : builder(context) {
            theModule = std::make_unique<Module>("test", context);
        }

        Type *getType(const string &target) {
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
        void pushBasicBlock(BasicBlock * p){
            this->bbs.push(p);
            this->builder.SetInsertPoint(this->bbs.top());
        }
        void popBasicBlock(){
            this->bbs.pop();
            this->builder.SetInsertPoint(this->bbs.top());
        }
    };

    bool isOutsideFunction(CodeContext &context) {
        return context.builder.GetInsertBlock()->getParent() == nullptr;
    }

    Value *LogErrorV(const string &str, Node *loc = nullptr) {
        if (loc) {
            fprintf(stderr, "Error: %s at %d:%d\n", str.c_str(), loc->line, loc->col);
        } else {
            fprintf(stderr, "Error: %s\n", str.c_str());
        }
        exit(1);
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
            if (isRef) {
                this->isMutable  = true;
                cout << "Gen IdentifierRef:" << name << endl;
                if (isOutsideFunction(context))
                    return LogErrorV("Can not ref var " + name + " out side function ", this);
                else if (Value *V = context.localSymbol[name])
                    if(1)
                        return V;
                    else
                        return context.builder.CreateLoad(V);
                else if (auto G = context.theModule->getGlobalVariable(name))
                    if(1)
                        return G;
                    else
                        return context.builder.CreateLoad(G);
                else
                    return LogErrorV("undefined variable " + name, this);
            } else
                return nullptr;
        } else
            return nullptr;
    }

    Value *BinaryOperatorExpr::codeGen(CodeContext &context) {
        cout << "Gen BinaryOperatorExpr" << endl;
        Value *L = lhs->codeGen(context);
        Value *R = rhs->codeGen(context);
        if (op != T_ASSIGN) {
            this->isMutable=false;
            if(lhs->isMutable)
                L = context.builder.CreateLoad(L);
            if(rhs->isMutable)
                R = context.builder.CreateLoad(R);
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
                    return LogErrorV("unknown...", this);
            }
        } else {
            if(!lhs->isMutable)
                return LogErrorV("Left value is not mutable",this);
            if(rhs->isMutable)
                R = context.builder.CreateLoad(R);
            context.builder.CreateStore(R,L);
            this->isMutable = true;
            return L;
        }
    }

    Value *VarDeclStmt::codeGen(CodeContext &context) {
        cout << "Gen VarDeclStmt " << "Type:" << type->name << " Name:" << id->name << endl;
        AllocaInst *p = nullptr;
        Value * q = nullptr;
        if (expr){
            q = expr->codeGen(context);
        }
        if (isRoot) {
            GlobalVariable *G = nullptr;
            if (type->name == "int") {
                context.theModule->getOrInsertGlobal(id->name, Type::getInt32Ty(context.context));
                G = context.theModule->getGlobalVariable(id->name);
                if(!q)
                    G->setInitializer(ConstantInt::get(Type::getInt32Ty(context.context), 0, true));
                else
                    G->setInitializer(dyn_cast<llvm::ConstantInt>(q));
            } else if (type->name == "double") {
                context.theModule->getOrInsertGlobal(id->name, Type::getDoubleTy(context.context));
                G = context.theModule->getGlobalVariable(id->name);
                if(!q)
                    G->setInitializer(ConstantInt::get(Type::getDoubleTy(context.context), 0, true));
                else
                    G->setInitializer(dyn_cast<llvm::ConstantFP>(q));
            }
        } else {
            if(context.localSymbol[id->name]){
                return LogErrorV("redefine var "+id->name,this);
            }
            if (type->name == "int") {
                p = context.builder.CreateAlloca(Type::getInt32Ty(context.context));
            } else if (type->name == "double") {
                p = context.builder.CreateAlloca(Type::getDoubleTy(context.context));
            }else{
                return LogErrorV("unknown type",this);
            }
            if(q)
                context.builder.CreateStore(q,p);
            context.localSymbol[id->name] = p;
        }

        cout << "end" << endl;
        return p;
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

    Value * CompoundStmt::codeGen(CodeContext &context) {
        if(stmts)
            return stmts->codeGen(context);
        else
            return nullptr;
    }

    Value *FuncDeclStmt::codeGen(CodeContext &context) {
        if(!isOutsideFunction(context)){
            return LogErrorV("can not define function inside function",this);
        }
        cout << "Gen FuncDeclStmt:" << endl;
        cout << "Function return type:" << type->name << endl;
        cout << "Function name:" << id->name << endl;
        cout << "Function args:" << endl;
        context.localSymbol.clear();
        std::vector<Type *> argTypes;
        std::vector<string> argNames;
        for (auto &arg:*args) {
            cout << "Type: " << arg->type->name << ",Name: " << arg->id->name << endl;
            argNames.push_back(arg->id->name);
            argTypes.push_back(context.getType(arg->type->name));
        }
        FunctionType *funcType = FunctionType::get(context.getType(type->name), argTypes, false);
        Function *func = Function::Create(funcType, GlobalValue::ExternalLinkage, id->name, context.theModule.get());
        BasicBlock *currentFuncStart = BasicBlock::Create(context.context, id->name + "_entry", func);
        context.pushBasicBlock(currentFuncStart);
        auto p_name = argNames.begin();
        for (auto &inner_arg:func->args()){
            AllocaInst * p = context.builder.CreateAlloca(inner_arg.getType());
            context.localSymbol[*p_name] = p;
            context.builder.CreateStore(&inner_arg,p);
            p_name++;
        }
        funcBody->codeGen(context);
        context.popBasicBlock();
        return func;
    }
    Value * ReturnStmt::codeGen(CodeContext &context) {

    }
}
