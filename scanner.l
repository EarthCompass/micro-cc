%{
#include <string>
#include <iostream>
#include "parser.h"
using namespace std;
%}
%option noyywrap
%%
[ \t\n]          ;
[0-9]+\.[0-9]+   { cout << "T_DOUBLE: " << yytext << endl;return T_DOUBLE;}
[0-9]+           { cout << "T_INTEGER: " << yytext << endl;return T_INTEGER;}
[a-zA-Z_][a-zA-Z0-9_]*     { cout << "T_IDENTIFIER: " << yytext << endl; return T_IDENTIFIER;}
"("              { cout << "T_LPAREN: " << yytext << endl; return T_LPAREN;}
")"              { cout << "T_RPAREN: " << yytext << endl; return T_RPAREN;}
"["              { cout << "T_LSQUBRACK: " << yytext << endl; return T_LSQUBRACK;}
"]"              { cout << "T_RSQUBRACK: " << yytext << endl; return T_RSQUBRACK;}
"{"              { cout << "T_LBRACE: " << yytext << endl; return T_LBRACE;}
"}"              { cout << "T_RBRACE: " << yytext << endl; return T_RBRACE;}
"+"              { cout << "T_ADD: " << yytext << endl; return T_ADD;}
"-"              { cout << "T_MINUS: " << yytext << endl; return T_MINUS;}
"/"              { cout << "T_DIV: " << yytext << endl; return T_DIV;}
"*"              { cout << "T_MUL: " << yytext << endl; return T_MUL;}
"%"              { cout << "T_MOD: " << yytext << endl; return T_MOD;}
";"              { cout << "T_SEMICOLON: " << yytext << endl; return T_SEMICOLON;}
.                { cout << "UNKNOWN" << endl;}
%%