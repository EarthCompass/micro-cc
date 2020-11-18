//
// Created by Frank Xiang on 2020/11/13.
//
#include "stdio.h"
#include "stdlib.h"
#include <iostream>
extern FILE *yyin;
extern int yyparse();
extern int yylex();

int main(int argc, const char *args[]) {
  if (argc > 1 && (yyin = fopen(args[1], "r")) == NULL) {
    fprintf(stderr, "can not open %s\n", args[1]);
    exit(1);
  }
  while (yylex()) {
  }
  return 0;
}

void yyerror(const char *s) {
  std::cout << "EEK, parse error!  Message: " << s << std::endl;
  // might as well halt now:
  exit(-1);
}