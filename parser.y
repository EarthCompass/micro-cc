%code requires { 
      #include <stdio.h>  
      #include "../Nodes.hpp" 
      using namespace microcc;
      using namespace std;
}
%code {
      
      extern int yylex (void);
      extern void yyerror(const char *s);
      Stmts * Mprogram;
      extern int yyparse();
}

%union{
      Stmts * stmts;
      Stmt * stmt;
      Expr * expr;
      IndentifierExpr * ident;
      std::string* string;
      int token;
}
/* %token NUM VAR  */
%token <string> T_INTEGER T_DOUBLE T_IDENTIFIER T_TYPE_INT T_TYPE_DOUBLE
%token <token> T_ADD T_MINUS T_DIV T_MUL T_MOD T_ASSIGN
%token T_LPAREN  T_RPAREN T_LSQUBRACK T_RSQUBRACK T_LBRACE T_RBRACE
%token T_SEMICOLON 

%left T_ADD T_MINUS
%left T_DIV T_MOD T_MUL  

%type <stmts> stmts
%type <stmt> stmt singleexprstmt val_dec
%type <ident> val_type
%type <expr> expr 
%start program
%%
program : stmts {Mprogram = $1;};
stmts : stmt {$$ = new Stmts(); $$->stmts.push_back(unique_ptr<Stmt>($1));}| stmts stmt {$1->stmts.push_back(unique_ptr<Stmt>($2));}
stmt : singleexprstmt {$$ = $1;} | val_dec
singleexprstmt : expr T_SEMICOLON {$$ = new SingleExprStmt(unique_ptr<Expr>($1));}
expr : T_INTEGER {$$ = new IntegerLiteralExpr(atol($1->c_str()));} |
      expr T_ADD expr { $$ = new BinaryOperatorExpr($2,unique_ptr<Expr>($1),unique_ptr<Expr>($3)); }|
      expr T_MOD expr { $$ = new BinaryOperatorExpr($2,unique_ptr<Expr>($1),unique_ptr<Expr>($3)); }|
      expr T_MINUS expr { $$ = new BinaryOperatorExpr($2,unique_ptr<Expr>($1),unique_ptr<Expr>($3)); }|
      expr T_MUL expr { $$ = new BinaryOperatorExpr($2,unique_ptr<Expr>($1),unique_ptr<Expr>($3)); } | 
      expr T_DIV expr { $$ = new BinaryOperatorExpr($2,unique_ptr<Expr>($1),unique_ptr<Expr>($3)); } |
      T_LPAREN expr T_RPAREN { $$ = $2; }
val_type : T_TYPE_INT {$$ = new IndentifierExpr($1,true);} | 
            T_TYPE_DOUBLE {$$ = new IndentifierExpr($1,true);}
val_dec : val_type T_IDENTIFIER T_SEMICOLON{ auto id = new IndentifierExpr($2,false); $$ = new VarDeclStmt(unique_ptr<IndentifierExpr>($1),unique_ptr<IndentifierExpr>(id));} 
%%