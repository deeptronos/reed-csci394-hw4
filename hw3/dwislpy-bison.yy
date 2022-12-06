%skeleton "lalr1.cc"
%require  "3.0"
%debug
%defines
%define api.namespace {DWISLPY}
%define api.parser.class {Parser}
%define parse.error verbose

%code requires{

    #include "dwislpy-ast.hh"

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
%token               INTC "int"
%token               STRC "str"
%token               ASGN "="
%token               PLUSEQUAL "+="
%token               MINUSEQUAL "-="
%token               LAND "and"
%token               LORR "or"
%token               LESS "<"
%token               LSQL "<="
%token               EQUL "=="
%token               DONT "not"
%token               DOIF "if"
%token               ELSE "else"
%token               DOWH "while"
%token               PLUS "+"
%token               MNUS "-"
%token               TMES "*"
%token               IDIV "//"
%token               IMOD "%"
%token               LPAR "("
%token               RPAR ")"
%token               DEFN "def"
%token               COLN ":"
%token               COMM ","
%token               RETN "return"
%token               NONE "None"
%token               TRUE "True"
%token               FALS "False"
%token <int>         NMBR
%token <std::string> NAME
%token <std::string> STRG

%type <Prgm_ptr> prgm
%type <Blck_ptr> blck
%type <Stmt_vec> stms
%type <Stmt_ptr> stmt
%type <Expn_ptr> expn
				//
%%

%start main;

%left PLUS MNUS;
%left TMES IMOD IDIV;

main:
  prgm {
      main.set($1);
  }
;

prgm:
   defs blck {
      Defs ds = $1;
      Blck_ptr b = $2;
      $$ = Prgm_ptr { new Prgm {ds, b, b->where()} };
  }
|  blck {
      Defs ds { };
      Blck_ptr b = $1;
      $$ = Prgm_ptr { new Prgm {ds, b, b->where()} };
  }
;

defs:
  defs defn {
      Defs ds = $1;
      ds.push_back($2);
      $$ = ss;
  }
  | defn {
      Defs ds { };
      ds.push_back($1);
      $$ = ss;
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
		//TODO Implement pattern matching for Defn (e.g.  DEF NAME LPAR Name RPAR COLN EOLN <nest>)
stmt:
  NAME ASGN expn EOLN {
      $$ = Asgn_ptr { new Asgn {$1,$3,lexer.locate(@2)} };
  }
| NAME PLUSEQUAL expn EOLN { // similar to token's name
      $$ = PlusEqual_ptr { new PlusEqual {$1,$3,lexer.locate(@2)} }; //$1 and $3 are name and expression
  }
| NAME MINUSEQUAL expn EOLN {
      $$ = MinusEqual_ptr { new MinusEqual {$1,$3,lexer.locate(@2)} };
  }
| DOIF LPAR expn RPAR COLN EOLN INDENT blck DEDENT EOLN ELSE INDENT blck DEDENT EOLN {
      $$ = Doif_ptr { new Doif {$3,$8,$13,lexer.locate(@1)} };
  }
| DOWH LPAR expn RPAR COLN EOLN INDENT blck DEDENT EOLN {
      $$ = Dowh_ptr { new Dowh {$3,$8,lexer.locate(@1)} };
  }
| PASS EOLN {
      $$ = Pass_ptr { new Pass {lexer.locate(@1)} };
  }
| PRNT LPAR expn RPAR EOLN {
      $$ = Prnt_ptr { new Prnt {$3,lexer.locate(@1)} };
  }
;

expn:
  expn PLUS expn {
      $$ = Plus_ptr { new Plus {$1,$3,lexer.locate(@2)} };
  }
| expn MNUS expn {
      $$ = Mnus_ptr { new Mnus {$1,$3,lexer.locate(@2)} };
  }
| expn TMES expn {
      $$ = Tmes_ptr { new Tmes {$1,$3,lexer.locate(@2)} };
  }
| expn IDIV expn {
      $$ = IDiv_ptr { new IDiv {$1,$3,lexer.locate(@2)} };
  }
| expn IMOD expn {
      $$ = IMod_ptr { new IMod {$1,$3,lexer.locate(@2)} };
  }
| expn LAND expn EOLN {
      $$ = Land_ptr { new Land {$1,$3,lexer.locate(@2)} };
  }
| expn LORR expn EOLN {
      $$ = Lorr_ptr { new Lorr {$1,$3,lexer.locate(@2)} };
  }
| expn LESS expn EOLN {
      $$ = Less_ptr { new Less {$1,$3,lexer.locate(@2)} };
  }
| expn LSQL expn EOLN {
      $$ = Lsql_ptr { new Lsql {$1,$3,lexer.locate(@2)} };
  }
| expn EQUL expn EOLN {
      $$ = Equal_ptr { new Equal {$1,$3,lexer.locate(@2)} };
  }
| expn DONT expn EOLN {
      $$ = Dont_ptr { new Dont {$1,$3,lexer.locate(@2)} };
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
| INTC LPAR expn RPAR {
      $$ = IntC_ptr { new IntC {$3,lexer.locate(@1)} };
  }
| STRC LPAR expn RPAR {
      $$ = StrC_ptr { new StrC {$3,lexer.locate(@1)} };
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
