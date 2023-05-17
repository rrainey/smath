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
 
%{

#ifdef DEBUG
#include <assert.h>
#include <stdio.h>

main()
{
	char	*p;

	assert(sizeof(int) >= sizeof(char *));

	while (p = (char *) yylex())
		printf ("%-10.10s is \"%s\"\n", p, yytext);
}

#define token(x)	(int) "x"

#else

#include <parser.h>
#define	token(x)	(yylval.y_int = x)

/*
 *  override the standard (f)lex input rule
 */

#if defined(FLEX_SCANNER)
#undef YY_INPUT
#define YY_INPUT(buf,res,max) { \
	char *p = buf; \
	while (max--) if ((*p++ = sgetc()) == EOF) break; \
	res = p - buf - 1; \
	if (res == 0) res = YY_NULL; \
	}
#else
#undef input
#define input() (((yytchar=yysptr>yysbuf?U(*--yysptr):sgetc())==10?(yylineno++,yytchar):yytchar)==EOF?0:yytchar)
#endif

static int	symbol(), blank_symbol(), percent(), constant(), string();
static int	definition();
static void	digest_comment();
extern int	sgetc();
extern struct s_node * SMathCreatePatternNode();
extern int	yyerror();

#endif
%}

letter			[a-zA-Z$]
decimal_digit		[0-9]
digit			[0-9a-fA-F]
digit_or_dot		[0-9a-fA-F.]
letter_or_digit		[0-9a-zA-Z$`]
pattern			[0-9a-zA-Z$`*]
end_of_line		[\n\f]
white_space		[ \t]
blank			[ \t]
minus			[\-]
plus			[\+]
percent			[%]
period			[\.]
other			.

%%

{letter}{letter_or_digit}*	return symbol();

{letter}{letter_or_digit}*_*	return blank_symbol();

_+				return blank_symbol();

{digit}{digit_or_dot}*		return constant();
{digit_or_dot}{digit_or_dot}+	return constant();

\?{pattern}+			definition();

{percent}+			return percent();
{percent}{decimal_digit}+	return percent();

"&&"				return token(AND);
"||"				return token(OR);

"[["				return token(LBRACE2);
"]]"				return token(RBRACE2);
"->"				return token(RULE);
"++"				return token(PLUSPLUS);
"+"				return token(PLUS);
"--"				return token(MINUSMINUS);
{minus}{white_space}+		return token(SUBTRACT);
"-"				return token(UMINUS);
"**"				return token(NON_COMMUT_TIMES);
"*"				return token(TIMES);
"~"				return token(TILDE);
"/@"				return token(MAP);
"//@"				return token(MAP_ALL);
"/:"				return token(TAG_SET);
"/;"				return token(CONDITION);
"/"				return token(DIVIDE);
"@@"				return token(APPLY);
"@"				return token(ATSIGN);
"!!"				return token(FACTORIAL2);
"!"				return token(BANG);
{period}{period}		return token(REPEATED);
{period}			return token(DOT);
{period}{period}{period}	return token(REPEATED_NULL);
"^^"				return token(BASE);
"^"				return token(POWER);
":>"				return token(RULE_DELAYED);
"::"				return token(MESSAGE_NAME);
"::="				return token(ALIAS);
":="				return token(SET_DELAYED);
"=."				return token(UNSET);
"=="				return token(EQUAL);
"!="				return token(UNEQUAL);
">>>"				return token(PUT_APPEND);
">>"				return token(PUT);
"<<"				return token(GET);
">="				return token(GREATER_EQUAL);
"<="				return token(LESS_EQUAL);
">"				return token(GREATER);
"<"				return token(LESS);
"="				return token(SET);
"##"				return token(SLOT_SEQUENCE);
"#"				return token(SLOT);
"'"				return token(DERIVATIVE);
";"				return token(COMPOUND_EXPR);

{white_space}+			;

{end_of_line}+			return token(END_OF_LINE);

"\""				return string();

"(\*"				digest_comment();

{other}				return token(yytext[0]);

%%

#include <smath.h>
#include <ctype.h>

extern int yylex();

#define lval(x)	(x == Symbol || x == Constant || x == Blank)
#define lvalx(x) (x == Symbol || x == Constant || x == Blank || x == ')' || \
	x == DOT)

extern struct s_const *ParseConstant();
extern struct s_const *StringConstant();

#include <string.h>

static void
digest_comment()
{
	register int c = yytext[1];

	while (c != EOF) {
		switch (c = input()) {
		case '*':
			if ((c = input()) == ')') {
				return;
			}
			break;
		}
	}

	yyerror ("End-of-File reached inside a comment");
	return;
}

static int
symbol()
{
	yylval.y_sym = SMathAddSymbol (yytext);

	return Symbol;
}

static int
percent()
{
	register char *p = yytext+1;
	extern int atoi();

	if (*p == '%' || *p == '\0') {
		yylval.y_int = - strlen(yytext);
	}
	else {
		yylval.y_int = atoi(p);
	}

	return OUT;
}

static int
blank_symbol()
{
	char	*p = strchr(yytext, '_'), *q;
	char	name[BUFSIZ];
	int	i, namelen;
	struct	s_symbol *type;

	if (!p) {
		fprintf (stderr, "assertion failed: strchr() returned NULL in blank symbol\n");
		exit (1);
	}

/*
 *  Count trailing underscores
 */

	for (i=0, q=p; *q == '_'; ++i, ++q)
		;

	switch (i) {
	case 1:
		type = BuiltinFn(BLANK);
		break;
	case 2:
		type = BuiltinFn(BLANK_SEQUENCE);
		break;
	case 3:
		type = BuiltinFn(BLANK_NULL_SEQUENCE);
		break;
	default:
		sprintf (name, "\'%s\' is not a valid symbol\n", yytext);
		yyerror (name);
		type = BuiltinFn(BLANK);
		break;
	}

	namelen = p - yytext;

	strncpy (name, yytext, namelen);
	name[namelen] = '\0';

	yylval.y_expr = SMathCreatePatternNode (name, type, (struct s_node *) NULL);
	return Blank;
}

static int
string()
{
	register char *p = yytext;

	register int c = yytext[0];

	while (c != EOF) {

		switch (c = input()) {

		case '"':
			*p = '\0';
			yylval.y_const = StringConstant(yytext);
			return Constant;
			break;

		case '\n':
		case '\f':
			yyerror("strings cannot span a line");
			return token(END_OF_LINE);
			break;

		case '\\':
			switch (c = input()) {
			case 'n':
				*p = '\n';
				break;
			case 't':
				*p = '\t';
				break;
			default:
				*p = c;
				break;
			}
			break;

		default:
			*p = c;
			break;
		}
		++ p;
	}

	yyerror("end-of-file encountered in a string");
	return EOF;
}

static int
constant()
{
	yylval.y_const = ParseConstant (yytext);
	return Constant;
}

static int
definition ()
{
	printf ("definition %s\n", yytext);
	return Constant;
}
