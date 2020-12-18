%{
#include <string>
#include <iostream>
#include "parser.h"
#define SAVE_TOKEN yylval.string = new string(yytext)
#define TOKEN(t) ( yylval.token = t)
using namespace std;
int colnum = 1;
#define YY_USER_ACTION {yylloc.first_line = yylineno; \
        yylloc.first_column = colnum;                 \
        colnum=colnum+yyleng;                         \
        yylloc.last_column=colnum;                    \
        yylloc.last_line = yylineno;}

%}
%option yylineno
%option noyywrap
%%
"int"            { cout << "T_TYPE_INT: "<< yytext <<endl;SAVE_TOKEN;return T_TYPE_INT;}
"double"         { cout << "T_TYPE_DOUBLE: "<< yytext <<endl;SAVE_TOKEN;return T_TYPE_DOUBLE;}
"return"         { cout << "T_RETURN" << yytext <<endl; return T_RETURN; }
"if"             { cout << "T_IF" << yytext <<endl; return T_IF; }
"else"           { cout << "T_ELSE" << yytext <<endl; return T_ELSE; }
[ \t]            ;
"\n"             { colnum = 1;}
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
">"              { cout << "T_GT: " << yytext << endl; return TOKEN(T_GT);}
">="             { cout << "T_GE: " << yytext << endl; return TOKEN(T_GE);}
"<"              { cout << "T_LT: " << yytext << endl; return TOKEN(T_LT);}
"<="             { cout << "T_LE: " << yytext << endl; return TOKEN(T_LE);}
"=="             { cout << "T_EQUAL: " << yytext << endl; return TOKEN(T_EQUAL);}
"="              { cout << "T_ASSIGN: " << yytext << endl; return TOKEN(T_ASSIGN);}
";"              { cout << "T_SEMICOLON: " << yytext << endl; return TOKEN(T_SEMICOLON);}
","              { cout << "T_COMMA: " << yytext << endl; return TOKEN(T_COMMA);}
.                { cout << "UNKNOWN" << endl;}

%%