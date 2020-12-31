//
// Created by Frank Xiang on 2020/11/13.
//
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <llvm/Support/CommandLine.h>
#include <llvm/Support/WithColor.h>
#include "Nodes.hpp"
#include "codegen.h"

extern FILE *yyin;

extern int yyparse();

extern int yylex();
extern microcc::Stmts *Mprogram;


cl::opt<bool> emitIR ("emit-ir", cl::desc("Print IR to stdout"));
cl::opt<bool> verbose ("v", cl::desc("Show more message"));
cl::opt<bool> printAST ("ast", cl::desc("Print AST to stdout"));
cl::opt<bool> printSymbol ("symbol", cl::desc("Print Symbol to stdout"));
cl::opt<string> outputFilename("o", cl::desc("Specify output filename, micro-cc will try to generate executable file using system cc if this is set"), cl::value_desc("filename"));
cl::opt<string> outputObjFilename("obj", cl::desc("Specify output obj filename"), cl::value_desc("filename"));
cl::opt<string> inputFilename(cl::Positional, cl::desc("<input file>"), cl::Required);

void version(raw_ostream & stream){
    WithColor(stream)<<"Micro C Compiler built by Ear7hC\n";
    cl::PrintVersionMessage();
}
int main(int argc, const char *argv[]) {
    cl::SetVersionPrinter(version);
    cl::ParseCommandLineOptions(argc, argv);
    if ((yyin = fopen(inputFilename.c_str(), "r")) == nullptr) {
        fprintf(stderr, "can not open %s\n", argv[1]);
        exit(1);
    }
    yyparse();
    for (auto &s:Mprogram->stmts){
        s->isRoot = true;
    }
    if(printAST){
        Mprogram->PrintAST(0);
    }
    CodeContext rootContext;
    rootContext.IRGen(*Mprogram);
    if(!outputFilename.empty()){
        rootContext.ObjectGen(outputObjFilename);
        if(!outputFilename.empty()){
            string s ="cc "+outputObjFilename+" -o "+outputFilename;
            system(s.c_str());
        }
    }

    return 0;
}

