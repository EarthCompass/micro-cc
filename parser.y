%code requires { 
      #include <stdio.h>  
      #include <stdlib.h>  
      #include "Nodes.hpp" 
      using namespace microcc;
      using namespace std;
}
%code {
      extern int yylex (void);
      // extern void yyerror(const char *s);
      Stmts * Mprogram;
      extern int yyparse();
      void yyerror(const char* s);
      #define LLOC(index) index.first_line,index.first_column
}

%union{
      Stmts * stmts;
      Stmt * stmt;
      Expr * expr;
      IdentifierExpr * ident;
      std::string* string;
      FuncDecArgsList* funcargs;
      CallArgs * callargs;
      int token;
}
%locations

/* %token NUM VAR  */
%token <string> T_INTEGER T_DOUBLE T_IDENTIFIER T_TYPE_INT T_TYPE_DOUBLE 
%token <token> T_ADD T_MINUS T_DIV T_MUL T_MOD T_ASSIGN T_GT T_GE T_LT T_LE T_EQUAL T_IF T_ELSE T_WHILE
%token T_LPAREN  T_RPAREN T_LSQUBRACK T_RSQUBRACK T_LBRACE T_RBRACE
%token T_SEMICOLON T_COMMA
%token T_RETURN

%left T_ADD T_MINUS
%left T_DIV T_MOD T_MUL  

%type <stmts> stmts
%type <stmt> stmt singleexprstmt val_dec_stmt func_dec_stmt compound_stmt return_stmt if_stmt while_stmt
%type <ident> val_type
%type <expr> expr call_expr
%type <token> cmp_operator 
%type <funcargs> func_args
%type <callargs>call_args 
%start program
%%
program : stmts {Mprogram = $1;};

stmts : /*blank*/{$$ = nullptr;} 
      | stmt {$$ = new Stmts(); $$->stmts.push_back(unique_ptr<Stmt>($1));}
      |            stmts stmt {$1->stmts.push_back(unique_ptr<Stmt>($2));}
            

stmt : singleexprstmt {$$ = $1;} 
      | val_dec_stmt 
      | compound_stmt 
      | func_dec_stmt 
      | return_stmt
      | if_stmt
      | while_stmt

singleexprstmt : expr T_SEMICOLON {$$ = new SingleExprStmt(unique_ptr<Expr>($1),LLOC(@2));}

compound_stmt : T_LBRACE stmts T_RBRACE
                  {$$ = new CompoundStmt(unique_ptr<Stmts>($2),LLOC(@2));}

return_stmt : T_RETURN expr T_SEMICOLON {$$ = new ReturnStmt(unique_ptr<Expr>($2),LLOC(@2));};

cmp_operator : T_GT 
            | T_GE 
            | T_LT 
            | T_LE 
            | T_EQUAL 

expr : T_INTEGER {$$ = new IntegerLiteralExpr(atol($1->c_str()),LLOC(@1));}  
      |      T_DOUBLE {$$ = new DoubleLiteralExpr(strtod($1->c_str(),nullptr),LLOC(@1));}  
      |      expr T_ADD expr { $$ = new BinaryOperatorExpr($2,unique_ptr<Expr>($1),unique_ptr<Expr>($3),LLOC(@2));}  
      |      expr T_MOD expr { $$ = new BinaryOperatorExpr($2,unique_ptr<Expr>($1),unique_ptr<Expr>($3),LLOC(@2)); } 
      |      expr T_MINUS expr { $$ = new BinaryOperatorExpr($2,unique_ptr<Expr>($1),unique_ptr<Expr>($3),LLOC(@2)); }   
      |      expr T_MUL expr { $$ = new BinaryOperatorExpr($2,unique_ptr<Expr>($1),unique_ptr<Expr>($3),LLOC(@2)); }  
      |      expr T_DIV expr { $$ = new BinaryOperatorExpr($2,unique_ptr<Expr>($1),unique_ptr<Expr>($3),LLOC(@2)); } 
      |      expr cmp_operator expr { $$ = new BinaryOperatorExpr($2,unique_ptr<Expr>($1),unique_ptr<Expr>($3),LLOC(@2)); }  
      |      T_LPAREN expr T_RPAREN { $$ = $2; }
      |      expr T_ASSIGN expr{ $1->isAssign = true;$$ = new BinaryOperatorExpr($2,unique_ptr<Expr>($1),unique_ptr<Expr>($3),LLOC(@2)); } 
      |      T_IDENTIFIER {$$ = new IdentifierExpr($1,false,LLOC(@1));}
      |      call_expr

val_type : T_TYPE_INT {$$ = new IdentifierExpr($1,true,LLOC(@1));} 
      |           T_TYPE_DOUBLE {$$ = new IdentifierExpr($1,true,LLOC(@1));}

val_dec_stmt : val_type T_IDENTIFIER T_SEMICOLON{ auto id = new IdentifierExpr($2,false,LLOC(@2)); $$ = new VarDeclStmt(unique_ptr<IdentifierExpr>($1),unique_ptr<IdentifierExpr>(id),nullptr,LLOC(@1));} 
      |            val_type T_IDENTIFIER T_ASSIGN expr T_SEMICOLON{ auto id = new IdentifierExpr($2,false,LLOC(@2));$$ = new VarDeclStmt(unique_ptr<IdentifierExpr>($1),unique_ptr<IdentifierExpr>(id),unique_ptr<Expr>($4),LLOC(@1));}


func_dec_stmt: val_type T_IDENTIFIER T_LPAREN func_args T_RPAREN compound_stmt 
            { auto id = new IdentifierExpr($2,false,LLOC(@2));
            $$ = new FuncDeclStmt(unique_ptr<IdentifierExpr>($1),unique_ptr<IdentifierExpr>(id),unique_ptr<FuncDecArgsList>($4),unique_ptr<CompoundStmt>((microcc::CompoundStmt *)$6),LLOC(@1)); }

func_args : /*blank*/ {$$ = new FuncDecArgsList();} 
      |      val_type T_IDENTIFIER {$$ = new FuncDecArgsList();
            auto id = new IdentifierExpr($2,false,LLOC(@2));
            auto arg = new VarDeclExpr(unique_ptr<IdentifierExpr>($1),unique_ptr<IdentifierExpr>(id),LLOC(@1));
            $$->push_back(unique_ptr<VarDeclExpr>(arg));}
      
      |      func_args T_COMMA val_type T_IDENTIFIER 
            {auto id = new IdentifierExpr($4,false,@4.first_line,@4.first_column);
            auto arg = new VarDeclExpr(unique_ptr<IdentifierExpr>($3),unique_ptr<IdentifierExpr>(id),LLOC(@3));
            $1->push_back(unique_ptr<VarDeclExpr>(arg));
            $$ = $1;}

call_args : /*blank*/ {$$ = new CallArgs();}
      |     expr {$$ = new CallArgs();$$->push_back(unique_ptr<Expr>($1));}
      |     call_args T_COMMA expr {$1->push_back(unique_ptr<Expr>($3));}
call_expr: T_IDENTIFIER T_LPAREN call_args T_RPAREN {auto callee = new IdentifierExpr($1,false,LLOC(@1));
            $$ = new CallExpr(unique_ptr<IdentifierExpr>(callee),unique_ptr<CallArgs>($3),LLOC(@1));}


if_stmt: T_IF T_LPAREN expr T_RPAREN compound_stmt T_ELSE compound_stmt {$$ = new IfStmt(unique_ptr<Expr>($3),unique_ptr<Stmt>($5),unique_ptr<Stmt>($7),LLOC(@1));}
      |  T_IF T_LPAREN expr T_RPAREN compound_stmt {$$ = new IfStmt(unique_ptr<Expr>($3),unique_ptr<Stmt>($5),nullptr,LLOC(@1));}

while_stmt: T_WHILE T_LPAREN expr T_RPAREN compound_stmt {$$ = new WhileStmt(unique_ptr<Expr>($3),unique_ptr<Stmt>($5),LLOC(@1));}

%%

void yyerror(const char* s) {
      std::cout << "0ops, parse error!  Message: " << s << " at "<<"line:"<<yylloc.first_line<<" col:"<<yylloc.first_column<<std::endl ;
      exit(-1);
}