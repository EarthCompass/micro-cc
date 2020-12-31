%{
#include <string>
#include <iostream>
#include <llvm/Support/CommandLine.h>
#include "parser.h"
#define SAVE_TOKEN yylval.string = new string(yytext)
#define TOKEN(t) ( yylval.token = t)
#define YY_USER_ACTION {yylloc.first_line = yylineno; \
        yylloc.first_column = colnum;                 \
        colnum=colnum+yyleng;                         \
        yylloc.last_column=colnum;                    \
        yylloc.last_line = yylineno;}
#define VERBOSE if(verbose)

using namespace std;
int colnum = 1;
extern llvm::cl::opt<bool> verbose;


%}
%option yylineno
%option noyywrap
%%
"int"            { VERBOSE cout << "T_TYPE_INT: "<< yytext <<endl;SAVE_TOKEN;return T_TYPE_INT;}
"double"         { VERBOSE cout << "T_TYPE_DOUBLE: "<< yytext <<endl;SAVE_TOKEN;return T_TYPE_DOUBLE;}
"return"         { VERBOSE cout << "T_RETURN" << yytext <<endl; return T_RETURN; }
"if"             { VERBOSE cout << "T_IF" << yytext <<endl; return T_IF; }
"else"           { VERBOSE cout << "T_ELSE" << yytext <<endl; return T_ELSE; }
"while"          { VERBOSE cout << "T_WHILE" << yytext <<endl; return T_WHILE; }
\".*\"           { VERBOSE cout << "T_STRING_LITERAL" << yytext <<endl;SAVE_TOKEN;return T_STRING_LITERAL; }
[ \t]            ;
"\n"             { VERBOSE colnum = 1;}
[0-9]+\.[0-9]+   { VERBOSE cout << "T_DOUBLE: " << yytext << endl;SAVE_TOKEN;return T_DOUBLE;}
[0-9]+           { VERBOSE cout << "T_INTEGER: " << yytext << endl;SAVE_TOKEN;return T_INTEGER;}
[a-zA-Z_][a-zA-Z0-9_]*     { VERBOSE cout << "T_IDENTIFIER: " << yytext << endl;SAVE_TOKEN;return T_IDENTIFIER;}
"("              { VERBOSE cout << "T_LPAREN: " << yytext << endl; return TOKEN(T_LPAREN);}
")"              { VERBOSE cout << "T_RPAREN: " << yytext << endl; return TOKEN(T_RPAREN);}
"["              { VERBOSE cout << "T_LSQUBRACK: " << yytext << endl; return TOKEN(T_LSQUBRACK);}
"]"              { VERBOSE cout << "T_RSQUBRACK: " << yytext << endl; return TOKEN(T_RSQUBRACK);}
"{"              { VERBOSE cout << "T_LBRACE: " << yytext << endl; return TOKEN(T_LBRACE);}
"}"              { VERBOSE cout << "T_RBRACE: " << yytext << endl; return TOKEN(T_RBRACE);}
"+"              { VERBOSE cout << "T_ADD: " << yytext << endl; return TOKEN(T_ADD);}
"-"              { VERBOSE cout << "T_MINUS: " << yytext << endl; return TOKEN(T_MINUS);}
"/"              { VERBOSE cout << "T_DIV: " << yytext << endl; return  TOKEN(T_DIV);}
"*"              { VERBOSE cout << "T_MUL: " << yytext << endl; return TOKEN(T_MUL);}
"%"              { VERBOSE cout << "T_MOD: " << yytext << endl; return TOKEN(T_MOD);}
">"              { VERBOSE cout << "T_GT: " << yytext << endl; return TOKEN(T_GT);}
">="             { VERBOSE cout << "T_GE: " << yytext << endl; return TOKEN(T_GE);}
"<"              { VERBOSE cout << "T_LT: " << yytext << endl; return TOKEN(T_LT);}
"<="             { VERBOSE cout << "T_LE: " << yytext << endl; return TOKEN(T_LE);}
"=="             { VERBOSE cout << "T_EQUAL: " << yytext << endl; return TOKEN(T_EQUAL);}
"="              { VERBOSE cout << "T_ASSIGN: " << yytext << endl; return TOKEN(T_ASSIGN);}
"="              { VERBOSE cout << "T_AND: " << yytext << endl; return TOKEN(T_AND);}
";"              { VERBOSE cout << "T_SEMICOLON: " << yytext << endl; return TOKEN(T_SEMICOLON);}
","              { VERBOSE cout << "T_COMMA: " << yytext << endl; return TOKEN(T_COMMA);}
.                { VERBOSE cout << "UNKNOWN" << endl;}

%%