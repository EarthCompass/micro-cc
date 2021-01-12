#include <llvm/IR/Value.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Target/TargetOptions.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/Host.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/TargetRegistry.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/Error.h>
#include "Nodes.hpp"
#include "parser.h"
#define VERBOSE if(verbose)

using namespace llvm;
using namespace std;

typedef std::map<std::string, AllocaInst *> localSymbolTable;
extern cl::opt<bool> emitIR;
extern cl::opt<bool> verbose;
extern cl::opt<bool> printSymbol;

namespace microcc {

    class CodeContext {
    public:
        LLVMContext context;
        unique_ptr<Module> theModule;
        IRBuilder<> builder;
        std::stack<BasicBlock *> bbs;
        std::vector<localSymbolTable *> localSymbolStack;
        // std::map<std::string, AllocaInst *> localSymbol;

        void IRGen(Stmts &root) {
            VERBOSE
            cout << "Generating IR code in context" << endl;
            // std::vector<Type *> sysArgs;
            BasicBlock *block = BasicBlock::Create(this->context, "entry");

            this->pushBasicBlock(block);
            FunctionType * printfType =  FunctionType::get(Type::getInt32Ty(context),true);
            Function::Create(printfType, GlobalValue::ExternalLinkage, "printf", this->theModule.get());
            FunctionType * scanfType =  FunctionType::get(Type::getInt32Ty(context),true);
            Function::Create(scanfType, GlobalValue::ExternalLinkage, "scanf", this->theModule.get());
            Value *p = root.codeGen(*this);
            if (!theModule->getFunction("main")) {
                cerr << "\"main\" function not found" << endl;
            }
            if(emitIR){
                cout << "IR code:" << endl;
                theModule->print(outs(), nullptr);
            }
        }

        void ObjectGen(std::string outputFileName) {
            InitializeNativeTarget();
            InitializeNativeTargetAsmPrinter();
            InitializeNativeTargetAsmParser();
            auto TargetTriple = llvm::sys::getDefaultTargetTriple();
            std::string Error;
            auto Target = TargetRegistry::lookupTarget(TargetTriple, Error);
            if (!Target) {
                errs() << Error;
                return;
            }
            auto CPU = "generic";
            auto Features = "";
            TargetOptions opt;
            auto RM = Optional<Reloc::Model>();
            auto TargetMachine = Target->createTargetMachine(TargetTriple, CPU, Features, opt, RM);
            theModule->setDataLayout(TargetMachine->createDataLayout());
            theModule->setTargetTriple(TargetTriple);
            auto Filename = outputFileName;
            std::error_code EC;
            raw_fd_ostream dest(Filename, EC, sys::fs::OF_None);
            legacy::PassManager pass;
            auto FileType = CGFT_ObjectFile;
            if (TargetMachine->addPassesToEmitFile(pass, dest, nullptr, FileType)) {
                errs() << "TargetMachine can't emit a file of this type";
                return;
            }
            pass.run(*theModule);
            dest.flush();
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

        inline void pushBasicBlock(BasicBlock *p) {
            this->bbs.push(p);
            this->builder.SetInsertPoint(this->bbs.top());
        }

        inline void popBasicBlock() {
            this->bbs.pop();
            this->builder.SetInsertPoint(this->bbs.top());
        }

        inline void pushLocalSymbolTable() {
            this->localSymbolStack.push_back(new localSymbolTable);
        }

        inline void popLocalSymbolTable() {
            if(printSymbol){
//                cout<<"function: "<<this->builder.GetInsertBlock()->getParent()->getName().str()<<endl;
                cout<<"------"<<endl;
                auto last = this->localSymbolStack.size() - 1;
                auto p = this->localSymbolStack[last];
                for (auto iter = p->rbegin();iter!=p->rend();iter++){
                    cout<<"local var :"<<iter->first<<", "<<iter->second<<endl;
                }
            }
            this->localSymbolStack.pop_back();
        }

        Value *findSymbolInStack(string &name) {
            for (auto p = this->localSymbolStack.size() - 1; ~p; p--) {
                if (this->localSymbolStack[p]->count(name))
                    return (*this->localSymbolStack[p])[name];
            }
            return nullptr;
        }

        inline localSymbolTable *getCurrentLocalSymbolTable() {
            return *(this->localSymbolStack.end() - 1);
        }
    };

    inline bool isOutsideFunction(CodeContext &context) {
        return context.builder.GetInsertBlock()->getParent() == nullptr;
    }

    Value *LogErrorV(const string &str, Node *loc = nullptr) {
        if (loc) {
//            fprintf(stderr, "Error: %s at %d:%d\n", str.c_str(), loc->line, loc->col);
            fprintf(stderr, "Error: %s at line %d\n", str.c_str(), loc->line);
        } else {
            fprintf(stderr, "Error: %s\n", str.c_str());
        }
        exit(1);
        return nullptr;
    }

    Value *IntegerLiteralExpr::codeGen(CodeContext &context) {
        VERBOSE
        cout << "Gen IntegerLiteral:" << value << endl;
        return ConstantInt::get(Type::getInt32Ty(context.context), value, true);
    }

    Value *DoubleLiteralExpr::codeGen(CodeContext &context) {
        VERBOSE
        cout << "Gen IntegerLiteral:" << value << endl;
        return ConstantFP::get(Type::getDoubleTy(context.context), value);
    }
    Value * StringLiteralExpr::codeGen(CodeContext &context) {
        VERBOSE
        cout << "Gen StringLiteralExpr:" << value << endl;
        return context.builder.CreateGlobalString(this->value, "str");
    }
    Value *IdentifierExpr::codeGen(CodeContext &context) {
        if (!isType) {
            if (isRef) {
                this->isMutable = true;
                VERBOSE
                cout << "Gen IdentifierRef:" << name << endl;
                if (isOutsideFunction(context))
                    return LogErrorV("Can not ref var " + name + " out side function ", this);
                else if (Value *V = context.findSymbolInStack(name))
                    return V;
                else if (auto G = context.theModule->getGlobalVariable(name))
                    return G;
                else
                    return LogErrorV("undefined variable " + name, this);
            } else
                return nullptr;
        } else
            return nullptr;
    }

    Value *BinaryOperatorExpr::codeGen(CodeContext &context) {
        VERBOSE
        cout << "Gen BinaryOperatorExpr" << endl;
        Value *L = lhs->codeGen(context);
        Value *R = rhs->codeGen(context);
        if (op != T_ASSIGN) {
            this->isMutable = false;
            if (lhs->isMutable)
                L = context.builder.CreateLoad(L);
            if (rhs->isMutable)
                R = context.builder.CreateLoad(R);
            bool hasDouble = false;

            // TODO: more type cast
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
            if (!lhs->isMutable)
                return LogErrorV("Left value is not mutable", this);
            if (rhs->isMutable)
                R = context.builder.CreateLoad(R);
            context.builder.CreateStore(R, L);
            this->isMutable = true;
            return L;
        }
    }

    Value *VarDeclStmt::codeGen(CodeContext &context) {
        VERBOSE
        cout << "Gen VarDeclStmt " << "Type:" << type->name << " Name:" << id->name << endl;
        AllocaInst *p = nullptr;
        Value *q = nullptr;
        if (expr) {
            q = expr->codeGen(context);
        }
        if(expr && expr->isMutable)
            q =  context.builder.CreateLoad(q);
        if (isRoot) {
            GlobalVariable *G = nullptr;
            if (type->name == "int") {
                if(context.theModule->getGlobalVariable(id->name)){
                    return LogErrorV("redefine global var " + id->name, this);
                }
                context.theModule->getOrInsertGlobal(id->name, Type::getInt32Ty(context.context));
                G = context.theModule->getGlobalVariable(id->name);
                if (!expr)
                    G->setInitializer(ConstantInt::get(Type::getInt32Ty(context.context), 0, true));
                else
                    G->setInitializer(dyn_cast<llvm::ConstantInt>(q));
            } else if (type->name == "double") {
                context.theModule->getOrInsertGlobal(id->name, Type::getDoubleTy(context.context));
                G = context.theModule->getGlobalVariable(id->name);
                if (!expr)
                    G->setInitializer(ConstantInt::get(Type::getDoubleTy(context.context), 0, true));
                else
                    G->setInitializer(dyn_cast<llvm::ConstantFP>(q));
            }
        } else {
            if (context.getCurrentLocalSymbolTable()->count(id->name)) {
                return LogErrorV("redefine var " + id->name, this);
            }
            if (type->name == "int") {
                p = context.builder.CreateAlloca(Type::getInt32Ty(context.context));
            } else if (type->name == "double") {
                p = context.builder.CreateAlloca(Type::getDoubleTy(context.context));
            } else {
                return LogErrorV("unknown type", this);
            }
            if (q)
                context.builder.CreateStore(q, p);
            (*context.getCurrentLocalSymbolTable())[id->name] = p;
        }
        VERBOSE
        cout << "end" << endl;
        return p;
    }

    Value *SingleExprStmt::codeGen(CodeContext &context) {
        VERBOSE
        cout << "Gen SingleExprStmt" << endl;
        return expr->codeGen(context);
    }

    Value *Stmts::codeGen(CodeContext &context) {
        VERBOSE
        cout << "Gen Stmts" << endl;
        Value *p = nullptr;
        for (auto &stmt:stmts) {
            if (stmt)
                p = stmt->codeGen(context);
        }
        return p;
    }

    Value *CompoundStmt::codeGen(CodeContext &context) {
        if (stmts) {
            if (!this->isFunctionBody)
                context.pushLocalSymbolTable();
            Value *p = stmts->codeGen(context);
            if (!this->isFunctionBody)
                context.popLocalSymbolTable();
            return p;
        } else
            return nullptr;
    }

    Value *FuncDeclStmt::codeGen(CodeContext &context) {
        if (!isOutsideFunction(context)) {
            return LogErrorV("can not define function inside function", this);
        }
        Function * getfunc = context.theModule->getFunction(id->name);
        if(getfunc)
            return LogErrorV("redefine function:"+id->name,this);
        VERBOSE{
            cout << "Gen FuncDeclStmt:" << endl;
            cout << "Function return type:" << type->name << endl;
            cout << "Function name:" << id->name << endl;
            cout << "Function args:" << endl;
        }
        // context.localSymbol.clear();
        std::vector<Type *> argTypes;
        std::vector<string> argNames;
        for (auto &arg:*args) {
            VERBOSE
            cout << "Type: " << arg->type->name << ",Name: " << arg->id->name << endl;
            argNames.push_back(arg->id->name);
            argTypes.push_back(context.getType(arg->type->name));
        }
        FunctionType *funcType = FunctionType::get(context.getType(type->name), argTypes, false);
        Function *func = Function::Create(funcType, GlobalValue::ExternalLinkage, id->name, context.theModule.get());
        BasicBlock *currentFuncStart = BasicBlock::Create(context.context, id->name + "_entry", func);
        context.pushLocalSymbolTable();
        context.pushBasicBlock(currentFuncStart);
        auto p_name = argNames.begin();
        for (auto &inner_arg:func->args()) {
            AllocaInst *p = context.builder.CreateAlloca(inner_arg.getType());
            (*context.getCurrentLocalSymbolTable())[*p_name] = p;
            context.builder.CreateStore(&inner_arg, p);
            p_name++;
        }
        funcBody->codeGen(context);
        context.popBasicBlock();
        context.popLocalSymbolTable();
        return func;
    }

    Value *ReturnStmt::codeGen(CodeContext &context) {
        VERBOSE
        cout << "Gen ReturnStmt" << endl;
        Value *ret = expr->codeGen(context);
        if (expr->isMutable) {
            ret = context.builder.CreateLoad(ret);
        }
        context.builder.CreateRet(ret);
        return nullptr;
    }

    Value *CallExpr::codeGen(CodeContext &context) {
        VERBOSE{
            cout << "Gen CallExpr" << endl;
            cout << "Callee: "<<callee->name<<endl;
        }
        Function * calleePtr = context.theModule->getFunction(callee->name);
        if(!calleePtr)
            return LogErrorV("call undefined function",this);
        else if(!calleePtr->isVarArg() && args->size()!=calleePtr->arg_size())
            return LogErrorV("function args count mismatch",this);
        else{
            vector<Value *> argsToPass;
            for (auto & argExpr:*args) {
                Value *p = argExpr->codeGen(context);
                if(argExpr->isMutable && !argExpr->isAssign){
                    p = context.builder.CreateLoad(p);
                }
                argsToPass.push_back(p);
            }
            return context.builder.CreateCall(calleePtr,argsToPass,"call");
        }
    }
    Value * IfStmt::codeGen(CodeContext &context) {
        Value * con = condition->codeGen(context);
        Function * currentFunction = context.builder.GetInsertBlock()->getParent();
        BasicBlock * trueBlock = BasicBlock::Create(context.context,"iftrue",currentFunction);
        BasicBlock * falseBlock = nullptr;
        if(elseStmts)
            falseBlock = BasicBlock::Create(context.context,"iffalse",currentFunction);
        BasicBlock * followBlock = BasicBlock::Create(context.context,"iffollow",currentFunction);
        if(elseStmts)
            context.builder.CreateCondBr(con,trueBlock,falseBlock);
        else
            context.builder.CreateCondBr(con,trueBlock,followBlock);
        context.pushBasicBlock(trueBlock);
        this->ifStmts->codeGen(context);
        context.builder.CreateBr(followBlock);
        context.popBasicBlock();
        if(elseStmts){
            context.pushBasicBlock(falseBlock);
            this->elseStmts->codeGen(context);
            context.builder.CreateBr(followBlock);
            context.popBasicBlock();
        }
        context.popBasicBlock();
        context.pushBasicBlock(followBlock);
        return nullptr;
    }
    Value * WhileStmt::codeGen(CodeContext &context) {
        Function * currentFunction = context.builder.GetInsertBlock()->getParent();
        BasicBlock * conBlock = BasicBlock::Create(context.context,"whilecondition",currentFunction);
        BasicBlock * bodyBlock = BasicBlock::Create(context.context,"whilebody",currentFunction);
        BasicBlock * followBlock = BasicBlock::Create(context.context,"whilefollow",currentFunction);
        //condition
        context.builder.CreateBr(conBlock);
        context.popBasicBlock();
        context.pushBasicBlock(conBlock);
        Value * con = condition->codeGen(context);
        context.builder.CreateCondBr(con,bodyBlock,followBlock);
        context.popBasicBlock();
        //while body
        context.pushBasicBlock(bodyBlock);
        body->codeGen(context);
        context.builder.CreateBr(conBlock);
        context.popBasicBlock();

        context.pushBasicBlock(followBlock);
        return nullptr;
    }
}
