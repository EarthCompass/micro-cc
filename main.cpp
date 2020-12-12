//
// Created by Frank Xiang on 2020/11/13.
//
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include "Nodes.hpp"
#include "codegen.h"
extern FILE *yyin;
extern int yyparse();
extern int yylex();
extern microcc::Stmts * Mprogram;

void yyerror(const char *s);
int main(int argc, const char *args[]) {
  if (argc > 1 && (yyin = fopen(args[1], "r")) == NULL) {
    fprintf(stderr, "can not open %s\n", args[1]);
    exit(1);
  }
  auto p = yyparse();
  Mprogram->PrintAST(0);
  CodeContext rootContext;
  rootContext.IRGen(*Mprogram);
  // while (yylex()) {
  // }
  return 0;
}

void yyerror(const char *s) {
  std::cout << "0ops, parse error!  Message: " << s << std::endl;
  // might as well halt now:
  exit(-1);
}