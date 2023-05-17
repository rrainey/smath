/*
 * The MIT License (MIT)
 * Copyright © 1996 Riley Rainey
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software 
 * and associated documentation files (the “Software”), to deal in the Software without restriction, 
 * including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, 
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or substantial 
 * portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT 
 * NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, 
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE 
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
 
 %union {
	struct s_symbol	*y_sym;
	struct s_node	*y_expr;
	struct s_const	*y_const;
	int		y_int;
	}

%{

#include <smath.h>
#ifdef MSDOS
#define alloca(s) _alloca(s)
#endif

extern struct s_node *SMathNodePrepend();
extern struct s_const *_s_const_minus_one;
extern int yyerror(), yylex();

%}

%token REPEATED_NULL, REPEATED, BASE
%token ALIAS, UNSET
%token SLOT_SEQUENCE, SLOT, DERIVATIVE
%token LIST, GENERAL, ON, OFF
%token END_OF_LINE
%token BLANK, BLANK_SEQUENCE, BLANK_NULL_SEQUENCE, PATTERN, OPTIONAL, DEFAULT
%token NULL_SYM, TRUE_SYM, FALSE_SYM
%token INTEGER_HEAD, REAL_HEAD, COMPLEX_HEAD, STRING_HEAD, HOLD

%token <y_sym> Symbol
%token <y_expr> Blank
%token <y_const> Constant
%token <y_int> OUT
%token '{', '}', '[', ']', ',', '(', ')'

%left COMPOUND_EXPR
%left GET, PUT, PUT_APPEND
%right SET, SET_DELAYED
%left TAG_SET, TAG_SET_DELAYED
%left CONDITION
%right RULE, RULE_DELAYED
%left  OR
%left  AND
%nonassoc NOT
%left  <y_int> EQUAL, UNEQUAL, GREATER_EQUAL, LESS_EQUAL, GREATER, LESS
%left PLUS, SUBTRACT
%right '(', '{'
%left TIMES, Symbol, Constant, Blank, OUT
%left DIVIDE
%left  UMINUS, UPLUS
%right  POWER
%nonassoc DOT
%nonassoc NON_COMMUT_TIMES
%nonassoc FACTORIAL, FACTORIAL2, BANG
%right MAP, MAP_ALL, APPLY
%left TILDE
%right ATSIGN
%right PLUSPLUS, MINUSMINUS
%left  PART, LBRACE2, RBRACE2
%nonassoc MESSAGE_NAME
%nonassoc ':'

%type <y_expr> input
%type <y_expr> expr_list, expr, list, optional_arg_list, arg_list, opt_expr
%type <y_int>  relop

%%

input	
	: expr_list END_OF_LINE
		{ _s_result = $1; yyerrok; yyclearin; }
	| error END_OF_LINE
		{ _s_result = NULL; yyerrok; yyclearin; }
	;

expr_list
	: /* empty */
		{ $$ = SMathCreateNode (BuiltinFn(NULL_SYM), NULL); }
	| expr
	| expr_list COMPOUND_EXPR opt_expr
		{ $$ = SMathCreateAssocNode (BuiltinFn(COMPOUND_EXPR), $1, $3); }
	;

expr
	: list
	| BANG expr %prec NOT
		{ $$ = SMathCreateNode (BuiltinFn(NOT), $2, NULL); }
	| expr AND expr
		{ $$ = SMathCreateAssocNode (BuiltinFn(AND), $1, $3); }
	| expr OR expr
		{ $$ = SMathCreateAssocNode (BuiltinFn(OR), $1, $3); }
	| expr relop expr %prec EQUAL
		{ $$ = SMathCreateAssocNode (BuiltinFn($2), $1, $3); }
	| expr PLUS expr
		{ $$ = SMathCreateAssocNode (BuiltinFn(PLUS), $1, $3); }
	| expr SUBTRACT expr
		{ $$ = SMathCreateAssocNode (BuiltinFn(SUBTRACT), $1, $3); }
	| expr TIMES expr
		{ $$ = SMathCreateAssocNode (BuiltinFn(TIMES), $1, $3); }
	| expr NON_COMMUT_TIMES expr
		{ $$ = SMathCreateAssocNode (BuiltinFn(NON_COMMUT_TIMES), $1, $3); }
	| expr expr %prec TIMES
		{ $$ = SMathCreateAssocNode (BuiltinFn(TIMES), $1, $2); }
	| expr DIVIDE expr
		{
		  struct s_node *p = SMathCreateIntegerNode (-1L);
		  $$ = SMathCreateNode (BuiltinFn(TIMES),
			$1,
			SMathCreateNode (BuiltinFn(POWER),
				$3,
				p,
				NULL),
			NULL);
		}
	| expr POWER expr
		{ $$ = SMathCreateNode (BuiltinFn(POWER), $1, $3, NULL); }
	| expr MAP expr
		{ $$ = SMathCreateNode (BuiltinFn(MAP), $1, $3, NULL); }
	| expr MAP_ALL expr
		{ $$ = SMathCreateNode (BuiltinFn(MAP), $1, $3, NULL); }
	| expr APPLY expr
		{ $$ = SMathCreateNode (BuiltinFn(MAP), $1, $3, NULL); }
	| expr FACTORIAL2
		{ $$ = SMathCreateNode (BuiltinFn(FACTORIAL2), $1, NULL); }
	| expr BANG %prec FACTORIAL
		{ $$ = SMathCreateNode (BuiltinFn(FACTORIAL), $1, NULL); }
	| expr TILDE expr TILDE expr
		{ $$ = SMathCreateNode ($3->tag, $1, $5, NULL); }
	| UMINUS expr
		{ struct s_node *p = SMathCreateIntegerNode (-1L);
		  $$ = SMathCreateNode (BuiltinFn(TIMES), p, $2, NULL);
		}
	| Symbol TAG_SET expr
		{ $$ = SMathCreateNode (BuiltinFn(TAG_SET), $1, $3, NULL); }
	| expr SET expr
		{ $$ = SMathCreateNode (BuiltinFn(SET), $1, $3, NULL); }
	| expr SET_DELAYED expr
		{ $$ = SMathCreateNode (BuiltinFn(SET_DELAYED),
			 $1, $3, NULL); }
	| expr CONDITION expr
		{ $$ = SMathCreateNode (BuiltinFn(CONDITION), $1, $3, NULL);}
	| expr MESSAGE_NAME Constant
		{
			$$ = SMathCreateNode (BuiltinFn(MESSAGE_NAME), $1, $3,
				NULL);
		}
	| expr MESSAGE_NAME Symbol
		{
			register struct s_node *p;
			p = SMathCreateStringNode ($3->s_name);
			$$ = SMathCreateNode (BuiltinFn(MESSAGE_NAME), $1, p,
				NULL);
		}
	| expr ':' expr
		{ $$ = SMathCreateNode (BuiltinFn(OPTIONAL), $1, $3, NULL); }
	| GET expr
		{ $$ = SMathCreateNode (BuiltinFn(GET), $2, NULL); }
	| expr PUT expr
		{ $$ = SMathCreateNode (BuiltinFn(PUT), $1, $3, NULL); }
	| expr PUT_APPEND expr
		{ $$ = SMathCreateNode (BuiltinFn(PUT_APPEND), $1, $3, NULL); }
	| expr RULE expr
		{ $$ = SMathCreateNode (BuiltinFn(RULE), $1, $3, NULL); }
	| expr RULE_DELAYED expr
		{ $$ = SMathCreateNode (BuiltinFn(RULE_DELAYED), $1, $3, NULL); }
	| '(' expr ')'
		{ $$ = $2; }
	| Symbol
		{ $$ = SMathCreateSymbolNode ($1); }
	| Blank
		{ $$ = $1; }
	| Blank DOT
		{ $$ = SMathCreateNode (BuiltinFn(OPTIONAL), $1, NULL); }
	| Constant
		{ $$ = SMathCreateConstantNode ($1); }
	| expr LBRACE2 arg_list RBRACE2
		{
			$3->tag = BuiltinFn(PART);
			$$ = SMathNodePrepend ($3, $1);
		}
	| Symbol '[' optional_arg_list ']'
		{ $3->tag = $1; $$ = $3; }
	| OUT
		{ $$ = SMathCreateNode (BuiltinFn(OUT),
			SMathCreateIntegerNode ((long) $1),
			NULL);
		}
	;

opt_expr
	: /* empty */
		{ $$ = SMathCreateNode (BuiltinFn(NULL_SYM), NULL); }
	| expr
	;

optional_arg_list
	: /* empty */
		{ $$ = SMathCreateNode (BuiltinFn(LIST), NULL); }
	| arg_list
	;

arg_list
	: expr
		{ $$ = SMathCreateNode (BuiltinFn(LIST), $1, NULL); }
	| arg_list ',' expr
		{ $$ = SMathNodeAddParameter ($1, $3); }
	;

list
	: '{' optional_arg_list '}'
		{ $$ = $2; }
	;

relop
	: EQUAL
	| UNEQUAL
	| GREATER_EQUAL
	| LESS_EQUAL
	| GREATER
	| LESS
	;
	
