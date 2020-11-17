%{
#include <stdio.h>  

extern int yylex (void);
extern void yyerror(const char *s);
extern int yyparse();
%}

%token NUM VAR PRICE OBJECT
%token T_INTEGER  T_IDENTIFIER T_DOUBLE
%token T_ADD  T_MINUS T_DIV T_MUL T_MOD
%token T_LPAREN  T_RPAREN T_LSQUBRACK T_RSQUBRACK T_LBRACE T_RBRACE
%token T_SEMICOLON

%%

description: object PRICE { printf("pass...\n"); }
      ;

object:  OBJECT
      ;

%%