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
 
#include <stdio.h>
#include <parser.h>
#include <smath.h>
#if !defined(MSDOS)
#include <sys/param.h>
#endif

extern YYSTYPE yylval;
extern int yyparse();
extern char *yytext;

struct s_node *SMathGet();

extern int fclose(), fputc();

char *_schar;
static char line[2048];
static int need_yyrestart = 0;

struct s_node *_s_path = (struct s_node *) NULL;

static int tty_errors = 0;
#ifdef BISON
extern void yyrestart();
#endif

struct s_node *
_SMathDollarPathRules (e, mflag)
struct s_node *e;
Boolean *mflag;
{
	SMathFreeExpr (e);
	*mflag = True;
	return (_s_path) ?
		SMathCopyExpr (_s_path) :
		SMathCreateSymbolNode (BuiltinFn(NULL_SYM));
}

struct s_node *
_SMathDollarPathLHS (e, mflag)
struct s_node *e;
Boolean *mflag;
{
	list_index_t i;

	if (e->tag != BuiltinFn(LIST)) {
		fprintf (stderr, "$Path must be set from a list\n");
		return e;
	}

	for (i=0; i<e->list_count; ++i) {
		if (!IsStringNode(e->list[i])) {
			fprintf (stderr, "each path must be a string\n");
			return e;
		}
	}

	if (_s_path) {
		SMathFreeExpr (_s_path);
	}

	_s_path = SMathCopyExpr (e);
	return e;
}

FILE *
SMathOpenFile (name, mode)
const char *name;
const char *mode;
{
	list_index_t i;
	char path[MAXPATHLEN];
	FILE *f;
	struct s_node *e = _s_path;
	int len;

	if (!_s_path) {
		strcpy (path, name);
		strcat (path, ".m");
		return fopen(name, mode);
	}

	for (i=0; i<e->list_count; ++i) {
		strcpy (path, ConstantVal(e->list[i], string_val));
		len = strlen(path);
		if (len > 0 && path[len-1] != '/') {
			strcat (path, "/");
		}
		strcat (path, name);
		strcat (path, ".m");
		if ((f = fopen(name, mode)) != (FILE *) NULL) {
			return f;
		}
	}

	return (FILE *) NULL;
}

struct s_node *
SMathParse(line)
char	*line;
{
	int	result;
	_schar = line;

#ifdef BISON
	if (need_yyrestart) {
		yyrestart(stdin);
	}
	else {
		need_yyrestart = 1;
	}
#endif

	result = yyparse();
	_schar = NULL;
	return result ? (struct s_node *) NULL : _s_result;
}


struct s_node *
_SMathGetRules (e, mflag)
struct s_node *e;
Boolean *mflag;
{
	char	filename[1024];

	if (e->list_count != 1) {
		return e;
	}

	if (IsStringNode(e->list[0])) {
		strcpy (filename, ConstantVal(e->list[0], string_val));
	}
	else if (IsSymbolNode(e->list[0])) {
		strcpy (filename, e->list[0]->tag->s_name);
	}
	else {
		return e;
	}

	SMathFreeExpr (e);

	return SMathGet (filename, mflag);

}

/* INTRINSA leaks_on_exit = none */
struct s_node *
SMathGet (filename, mflag)
char * filename;
Boolean *mflag;
{
	char	line[1024];
	char	name[MAXPATHLEN];
	FILE	*file;
	struct s_node *e1 = (struct s_node *) NULL;
	Boolean mflag1;
	long	line_no = 1;

	tty_errors ++;

	if ((file = fopen(filename, "r")) == (FILE *) NULL) {
		strcpy (name, filename);
		strcat (name, ".m");
		if ((file = SMathOpenFile(name, "r")) == (FILE *) NULL) {
			fprintf (stderr, "unable to open %s\n", filename);
			return SMathCreateSymbolNode(BuiltinFn(NULL_SYM));
		}
	}

	while (fgets(line, sizeof(line), file) != (char *) NULL) {

	    if (strlen(line) > 1) {

		mflag1 = False;
		if ((e1 = SMathParse(line))) {
			e1 = SMathEvaluate(&e1, e1, &mflag1, NoContext);
		}
		else {
			fprintf (stderr, "%s: %ld: syntax error:\n\t%s\n",
				filename, line_no, line);
		}

	    }

	    line_no ++;

	}

	tty_errors--;

	if (e1 == (struct s_node *) NULL) {
		e1 = SMathCreateSymbolNode(BuiltinFn(NULL_SYM));
	}

	fclose (file);
	*mflag = True;
	return e1;
}

int
sgetc()
{
	int	n = *_schar;

	if (n) _schar++;

	return (n == 0) ? EOF : n;
}		

int
yyerror(s)
char	*s;
{
	int	i;

	if (tty_errors != 0)
		return 0;

	fputc ('\t', stderr);
	for (i=0; i<(_schar - line - 1); ++i)
		fputc (' ', stderr);
	fprintf (stderr, "^\n%s\n", s);
	return 0;
}
