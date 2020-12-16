//
// Created by Frank Xiang on 2020/11/13.
//
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <llvm/Support/TargetSelect.h>
#include "Nodes.hpp"
#include "codegen.h"

extern FILE *yyin;

extern int yyparse();

extern int yylex();
extern microcc::Stmts *Mprogram;


int main(int argc, const char *args[]) {
    if (argc > 1 && (yyin = fopen(args[1], "r")) == NULL) {
        fprintf(stderr, "can not open %s\n", args[1]);
        exit(1);
    }
    InitializeNativeTarget();
    InitializeNativeTargetAsmPrinter();
    InitializeNativeTargetAsmParser();
    yyparse();
    for (auto &s:Mprogram->stmts){
        s->isRoot = true;
    }
    Mprogram->PrintAST(0);
    CodeContext rootContext;
    rootContext.IRGen(*Mprogram);
    // while (yylex()) {
    // }
    return 0;
}

