%skeleton "lalr1.cc"
%require  "3.0"
%debug 
%defines 
%define api.namespace {DWISLPY}
%define api.parser.class {Parser}
%define parse.error verbose
    
%code requires{
    
    #include "dwislpy-ast.hh"
    #include "dwislpy-check.hh"

    namespace DWISLPY {
        class Driver;
        class Lexer;
    }

    #define YY_NULLPTR nullptr

}

%parse-param { Lexer  &lexer }
%parse-param { Driver &main  }
    
%code{

    #include <sstream>
    #include "dwislpy-util.hh"    
    #include "dwislpy-main.hh"

    #undef yylex
    #define yylex lexer.yylex
    
}


%define api.value.type variant
%define parse.assert
%define api.token.prefix {Token_}

%locations

%token               EOFL  0  
%token               EOLN
%token               INDT
%token               DEDT
%token               PASS "pass"
%token               PRNT "print"
%token               INPT "input"
%token               BOOL "bool"
%token               INTC "int"
%token               STRC "str"
%token               DEFN "def"
%token               RTRN "return"
%token               AND  "and"
%token               OR   "or"
%token               NOT  "not"
%token               IFTN "if"
%token               ELSE "else"
%token               WHLE "while"
%token               ARRW "->"
%token               ASGN "="
%token               PLUS "+"
%token               MNUS "-"
%token               TMES "*"
%token               IDIV "//"
%token               IMOD "%"
%token               LSEQ "<="
%token               LESS "<"
%token               EQUL "=="
%token               LPAR "(" 
%token               RPAR ")"
%token               CMMA ","
%token               COLN ":"
%token               NONE "None"
%token               TRUE "True"
%token               FALS "False"
%token <int>         NMBR
%token <std::string> NAME
%token <std::string> STRG

%type <Prgm_ptr> prgm
%type <Defs>     defs
%type <Defn_ptr> defn
%type <SymT>     fmls
%type <Type>     type
%type <Blck_ptr> nest
%type <Blck_ptr> blck
%type <Stmt_vec> stms
%type <Stmt_ptr> stmt
%type <Expn_ptr> expn

%%

%start main;

%left AND;
%nonassoc LESS;
%left PLUS MNUS;
%left TMES IMOD IDIV;
    
main:
  prgm {
      main.set($1);
  }
;

prgm:
  blck {
      Defs ds { };
      Blck_ptr b = $1; 
      $$ = Prgm_ptr { new Prgm {ds, b, b->where()} };
  }   
| defs blck {
      Defs     ds = $1;
      Blck_ptr b  = $2; 
      $$ = Prgm_ptr { new Prgm {ds, b, b->where()} };
  }
;

defs: defn {
      Defs ds { };
      Defn_ptr d = $1;
      ds[d->name] = d;
      $$ = ds;
  }
| defs defn {
      Defs ds = $1;
      Defn_ptr d = $2;
      ds[d->name] = d;
      $$ = ds;
  }
;

defn:
  DEFN NAME LPAR RPAR ARRW type COLN EOLN nest {
    SymT ps { };
    $$ = Defn_ptr { new Defn {$2, ps, $6, $9, lexer.locate(@1)} };
  }    
| DEFN NAME LPAR RPAR COLN EOLN nest {
    SymT ps { };
    $$ = Defn_ptr { new Defn {$2, ps, NoneTy {}, $7, lexer.locate(@1)} };
  }    
| DEFN NAME LPAR fmls RPAR ARRW type COLN EOLN nest {
    $$ = Defn_ptr { new Defn {$2, $4, $7, $10, lexer.locate(@1)} };
  }
| DEFN NAME LPAR fmls RPAR COLN EOLN nest {
    $$ = Defn_ptr { new Defn {$2, $4, NoneTy {}, $8, lexer.locate(@1)} };
  }
;

fmls:
  NAME COLN type {
    SymT ps { };
    ps.add_frml($1,$3);
    $$ = ps;
  }
| fmls CMMA NAME COLN type {
    SymT ps = $1;
    ps.add_frml($3,$5);
    $$ = ps;
  }
;

type:
  INTC {
    $$ = IntTy {};
  }
| STRC {
    $$ = StrTy {};
  }
| BOOL {
    $$ = BoolTy {};
  }
| NONE {
    $$ = NoneTy {};
  }
;
        
nest:
  INDT blck DEDT {
    $$ = $2;
  }
;

blck:
  stms {
      Stmt_vec ss = $1;
      $$ = Blck_ptr { new Blck {ss, ss[0]->where()} };
  }
;

stms:
  stms stmt {
      Stmt_vec ss = $1;
      ss.push_back($2);
      $$ = ss;
  }
| stmt {
      Stmt_vec ss { };
      ss.push_back($1);
      $$ = ss;
  }
;
  
stmt: 
  NAME COLN type ASGN expn EOLN {
      $$ = Ntro_ptr { new Ntro {$1,$3,$5,lexer.locate(@2)} };
  }
| NAME ASGN expn EOLN {
      $$ = Asgn_ptr { new Asgn {$1,$3,lexer.locate(@2)} };
  }
| PASS EOLN {
      $$ = Pass_ptr { new Pass {lexer.locate(@1)} };
  }
| PRNT LPAR expn RPAR EOLN {
      $$ = Prnt_ptr { new Prnt {$3,lexer.locate(@1)} };
  }
| RTRN expn EOLN {
      $$ = FRtn_ptr { new FRtn {$2,lexer.locate(@1)} };
  }
| RTRN EOLN {
      $$ = PRtn_ptr { new PRtn {lexer.locate(@1)} };
  }
;

expn:
  expn PLUS expn {
      $$ = Plus_ptr { new Plus {$1,$3,lexer.locate(@2)} };
  }
| expn LESS expn {
      $$ = Less_ptr { new Less {$1,$3,lexer.locate(@2)} };
  }
| expn AND expn {
      $$ = And_ptr { new And {$1,$3,lexer.locate(@2)} };
  }
| NMBR {
      $$ = Ltrl_ptr { new Ltrl {Valu {$1},lexer.locate(@1)} };
  }
| STRG {
      $$ = Ltrl_ptr { new Ltrl {Valu {de_escape($1)},lexer.locate(@1)} };
  }
| TRUE {
      $$ = Ltrl_ptr { new Ltrl {Valu {true},lexer.locate(@1)} };
  }
| FALS {
      $$ = Ltrl_ptr { new Ltrl {Valu {false},lexer.locate(@1)} };
  }
| NONE {
      $$ = Ltrl_ptr { new Ltrl {Valu {None},lexer.locate(@1)} };
  }    
| INPT LPAR expn RPAR {
      $$ = Inpt_ptr { new Inpt {$3,lexer.locate(@1)} };
  }
| NAME {
      $$ = Lkup_ptr { new Lkup {$1,lexer.locate(@1)} };
  }
| LPAR expn RPAR {
      $$ = $2;
  }
;

%%
       
void DWISLPY::Parser::error(const location_type &loc, const std::string &msg) {
    throw DwislpyError { lexer.locate(loc), msg };
}
