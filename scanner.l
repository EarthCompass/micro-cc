%{
#include <string>
#include <iostream>
#include "parser.h"
#define SAVE_TOKEN yylval.string = new string(yytext)
#define TOKEN(t) ( yylval.token = t)
using namespace std;
%}
%option noyywrap
%%
[ \t\n]          ;
[0-9]+\.[0-9]+   { cout << "T_DOUBLE: " << yytext << endl;SAVE_TOKEN;return T_DOUBLE;}
[0-9]+           { cout << "T_INTEGER: " << yytext << endl;SAVE_TOKEN;return T_INTEGER;}
[a-zA-Z_][a-zA-Z0-9_]*     { cout << "T_IDENTIFIER: " << yytext << endl;SAVE_TOKEN;return T_IDENTIFIER;}
"("              { cout << "T_LPAREN: " << yytext << endl; return TOKEN(T_LPAREN);}
")"              { cout << "T_RPAREN: " << yytext << endl; return TOKEN(T_RPAREN);}
"["              { cout << "T_LSQUBRACK: " << yytext << endl; return TOKEN(T_LSQUBRACK);}
"]"              { cout << "T_RSQUBRACK: " << yytext << endl; return TOKEN(T_RSQUBRACK);}
"{"              { cout << "T_LBRACE: " << yytext << endl; return TOKEN(T_LBRACE);}
"}"              { cout << "T_RBRACE: " << yytext << endl; return TOKEN(T_RBRACE);}
"+"              { cout << "T_ADD: " << yytext << endl; return TOKEN(T_ADD);}
"-"              { cout << "T_MINUS: " << yytext << endl; return TOKEN(T_MINUS);}
"/"              { cout << "T_DIV: " << yytext << endl; return  TOKEN(T_DIV);}
"*"              { cout << "T_MUL: " << yytext << endl; return TOKEN(T_MUL);}
"%"              { cout << "T_MOD: " << yytext << endl; return TOKEN(T_MOD);}
";"              { cout << "T_SEMICOLON: " << yytext << endl; return TOKEN(T_SEMICOLON);}
.                { cout << "UNKNOWN" << endl;}
%%