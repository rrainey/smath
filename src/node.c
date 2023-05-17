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
 
#include <smath.h>
#include <stdio.h>
#include <malloc.h>
#ifdef USE_STDARG
#include <stdarg.h>
#else
#include <varargs.h>
#endif
#include <parser.h>

#ifndef DEFAULT_LIST_LENGTH
#define DEFAULT_LIST_LENGTH	4
#endif

extern void FPrintConstant PARAMS((struct s_const *, FILE *));
extern struct s_const *CopyConstant PARAMS((const struct s_const *));

#define MAX_ARGS	16

static char *errmsg = "fatal memory allocation error\n";

/*
 *  SMathCreateNode (struct s_symbol *, struct s_node *, ...);
 */

#ifdef USE_STDARG
struct s_node *
SMathCreateNode (struct s_symbol *tag, ...)
#else
struct s_node *
SMathCreateNode (va_alist)
va_dcl
#endif
{
	va_list		ap;
	register struct s_node	*new, *p;
	struct s_node   *arg[MAX_ARGS];
	size_t		size;
	int		i, j, n;
#if defined(USE_STDARG)
	va_start(ap, tag);
#else
	register struct s_symbol *tag;

	va_start(ap);

	tag = va_arg (ap, struct s_symbol *);
#endif
	for (i=0;(p = va_arg (ap, struct s_node *)) != 0;) {
		arg[i++] = p;
		if (i == 16) {
			fprintf (stderr,
			    "max arg count exceeded\n");
			return (struct s_node *) NULL;
		}
	}

	n = i;

	if (n == 0)
		n = DEFAULT_LIST_LENGTH;

	size = n * sizeof(struct s_node *);

	if ((new=(struct s_node *)malloc(sizeof(struct s_node)))
		== (struct s_node *) NULL) {
		fprintf (stderr, "%s", errmsg);
		exit (1);
	}

#ifdef DEBUG
	fprintf (stderr, "creating %s node with %d leaves (%d max)\n",
		tag->s_name, i, n);
#endif

	new->tag = tag;
	new->list_count = i;
	new->list_max = n;
	new->list = (struct s_node **) malloc (size);
	if (new->list == (struct s_node **) NULL) {
		fprintf (stderr, "%s", errmsg);
		exit (1);
	}
	for (j=0; j<i; ++j)
		new->list[j] = arg[j];

	va_end (ap);
	return new;
}

struct s_node *
SMathCreateNodeBySize(tag, list_max)
struct s_symbol	*tag;
list_index_t	list_max;
{
	size_t	size;
	register struct s_node *new;

	size = list_max * sizeof(struct s_node *);

	if ((new=(struct s_node *)malloc(sizeof(struct s_node)))
		== (struct s_node *) NULL) {
		fprintf (stderr, "%s", errmsg);
		exit (1);
	}

#ifdef DEBUG
	fprintf (stderr, "creating %s node with %d leaves (%d max)\n",
		tag->s_name, list_max, list_max);
#endif

	new->tag = tag;
	new->list_count = 0;
	new->list_max = list_max;
	new->list = (struct s_node **) malloc (size);
	if (new->list == (struct s_node **) NULL) {
		fprintf (stderr, "%s", errmsg);
		exit (1);
	}

	return new;
}

struct s_node *
SMathCopyExpr(e)
struct s_node *e;
{
	register list_index_t	i;
	register struct s_node	*new;
	size_t	size;

	size = e->list_max * sizeof(struct s_node *);

	new = (struct s_node *) malloc (sizeof(struct s_node));
	if (!new) {
		fprintf (stderr, "%s", errmsg);
		exit (1);
	}

#ifdef DEBUG
	fprintf (stderr, "copying %s node with %d leaves (%d max)\n",
		e->tag ? e->tag->s_name : "(constant)",
		e->list_count, e->list_max);
#endif

	new->tag = e->tag;
	new->list_count = e->list_count;
	new->list_max = e->list_max;

	if (new->list_count > new->list_max) {
		fprintf (stderr, "Assertion failed: list_count > list_max\n");
		exit (1);
	}

	if (new->list_max > 0) {
		new->list = (struct s_node **) malloc (e->list_max * sizeof(struct s_node *));
	}
	else if (!e->tag) {
		new->list = (struct s_node **) CopyConstant
			((struct s_const *) e->list);
		new->list_count = new->list_max = 0;
	}
	else {
		new->list_count = 0;
		new->list = (struct s_node **) NULL;
	}

	if (new->list == (struct s_node **) NULL && new->list_max > 0) {
		fprintf (stderr, "%s", errmsg);
		exit (1);
	}

	for (i=0; i<new->list_count; ++i) {
		new->list[i] = SMathCopyExpr(e->list[i]);
	}

	return new;
}

struct s_node *
SMathCreateSymbolNode (symbol)
struct s_symbol *symbol;
{
	register struct s_node *new;

#ifdef DEBUG
	fprintf (stderr, "SMathCreateSymbolNode(%s)\n", symbol->s_name);
#endif
	
	if ((new=(struct s_node *)malloc(sizeof(struct s_node)))
		== (struct s_node *) NULL) {
		fprintf (stderr, "%s", errmsg);
		exit (1);
	}
	(symbol->s_refcnt)++;
	new->tag = symbol;
	new->list_count = new->list_max = 0;
	new->list = (struct s_node **) NULL;

	return new;
}

struct s_node *
SMathCreateConstantNode (c)
struct s_const *c;
{
	register struct s_node *new;

#ifdef DEBUG
	fprintf (stderr, "SMathCreateConstantNode()\n");
#endif
	
	if ((new=(struct s_node *)malloc(sizeof(struct s_node)))
		== (struct s_node *) NULL) {
		fprintf (stderr, "%s", errmsg);
		exit (1);
	}
	new->tag = NULL;
	new->list_count = new->list_max = 0;
	new->list = (struct s_node **) c;

	return new;
}

struct s_node *
SMathCreateStringNode (s)
const char 	*s;
{
	register struct s_const *c;

	c = (struct s_const *) malloc(sizeof(struct s_const));
	if (!c) {
		fprintf (stderr, "%s", errmsg);
		exit (1);
	}
	c->type = CONST_STRING;
	c->u.string_val = strdup (s);
	return SMathCreateConstantNode (c);
}

struct s_node *
SMathCreateIntegerNode (i)
const long	i;
{
	register struct s_const *c;

	c = (struct s_const *) malloc(sizeof(struct s_const));
	if (!c) {
		fprintf (stderr, "%s", errmsg);
		exit(1);
	}
	c->type = CONST_LONG;
	c->u.long_val = i;
	return SMathCreateConstantNode (c);
}


struct s_node *
SMathNodeAddParameter (node, p)
struct s_node *node;
struct s_node *p;
{
	struct s_node **new;
	size_t	size;

	if (node->list_count == node->list_max) {
		node->list_max <<= 1;
		size = node->list_max * sizeof(struct s_node *);

		if ((new = (struct s_node **)realloc((malloc_t)node->list, size))
			== (struct s_node **) NULL) {
			fprintf (stderr, "%s", errmsg);
			exit (1);
		}
		node->list = new;
	}

	node->list[node->list_count++] = p;

#ifdef DEBUG
	fprintf (stderr, "SMathNodeAddParameter(%s, ", node->tag->s_name);
	SMathFPrintExpr(p, (struct s_node *) NULL, stderr);
	fprintf (stderr, ");\nnode now has %d leaves (%d max)\n",
		node->list_count, node->list_max);
#endif

	return node;
}

struct s_node *
SMathNodePrepend (node, p)
struct s_node *node;
struct s_node *p;
{
	struct s_node **new;
	size_t	size;
	register list_index_t i;

	if (node->list_count == node->list_max) {
		node->list_max <<= 1;
		size = node->list_max * sizeof(struct s_node *);

		if ((new = (struct s_node **)realloc((malloc_t)node->list, size))
			== (struct s_node **) NULL) {
			fprintf (stderr, "%s", errmsg);
			exit (1);
		}
		node->list = new;
	}

	for (i=node->list_count; ; --i) {
		node->list[i] = node->list[i-1];
		if (i == 0)
			break;
	}

	++ node->list_count;

	node->list[0] = p;

	return node;
}

struct s_node *
SMathNodeRemoveParameter (e, id)
struct s_node *e;
list_index_t	id;
{
	register list_index_t i;

	SMathFreeExpr (e->list[id]);

	for (i=id+1; i<e->list_count; ++i) {
		e->list[i-1] = e->list[i];
	}

	-- e->list_count;

	return e;
}

struct s_node *
SMathNodeRemoveParameters (e, id, count)
struct s_node	*e;
list_index_t	*id;
list_index_t	count;
{
	Boolean modified = True;
	register list_index_t i, tmp;

	while (modified) {
		modified = False;
		for (i=0; i<count-1; ++i) {
			if (id[i] > id[i+1]) {
				tmp = id[i];
				id[i] = id[i+1];
				id[i+1] = tmp;
				modified = True;
			}
		}
	}

	for (i=0; i<count; ++i) {
		(void) SMathNodeRemoveParameter (e, id[i]-i);
	}

	return e;
}

static void
FPrintContext (tag, f)
struct s_symbol *tag;
FILE *f;
{
	if (tag->s_context) {
		FPrintContext (tag->s_context, f);
	}
	fprintf (f, "%s", tag->s_name);
}

void
SMathFPrintExpr(p, prev, f)
struct s_node *p;
struct s_node *prev;
FILE	*f;
{
	register list_index_t i;

	if (!p) {
		fprintf (f, "(null-node)");
		return;
	}

	if (!p->tag) {
		FPrintConstant ((struct s_const *) p->list, f);
		return;
	}

	if (p->tag->s_type == TYPE_SYMBOL ||
	    p->tag->s_type == TYPE_ATTRIBUTE) {
		if (p->list_count) {
			fprintf (f, "%s[", p->tag->s_name);
			for (i=0; i<p->list_count; ++i) {
				SMathFPrintExpr (p->list[i], p, f);
				if (i < p->list_count-1)
					fprintf (f, ", ");
			}
			fprintf (f, "]");
		}
		else {
			fprintf (f, "%s", p->tag->s_name);
		}
	}
	else if (p->tag->s_type == TYPE_BUILTIN) {
		if (p->tag->s_print) {
			(*p->tag->s_print)(p, prev, f);
		}
		else {
			fprintf (f, "%s[", p->tag->s_name);
			for (i=0; i<p->list_count; ++i) {
				SMathFPrintExpr (p->list[i], p, f);
				if (i < p->list_count-1)
					fprintf (f, ", ");
			}
			fprintf (f, "]");
		}
	}
	else if (p->tag->s_type == TYPE_CONTEXT) {
		if (p->tag->s_context) {
			FPrintContext(p->tag->s_context, f);
		}
		fprintf (f, "%s", p->tag->s_name);
	}
}

void
SMathFSMathPrintInfix(p, prev, f)
struct s_node *p;
struct s_node *prev;
FILE	*f;
{

	char	*c = "*unknown*";
	register list_index_t	i;

	if (p->list_count == 0) {
		fprintf (f, "%s", p->tag->s_name);
		return;
	}

	if (prev && (prev->tag->s_prec > p->tag->s_prec))
		fprintf (f, "(");

	if (p->tag == BuiltinFn(PLUS)) {
		c = " + ";
	}
	else if (p->tag == BuiltinFn(SUBTRACT)) {
		c = " - ";
	}
	else if (p->tag == BuiltinFn(TIMES)) {
		c = " ";
	}
	else if (p->tag == BuiltinFn(DIVIDE)) {
		c = " / ";
	}
	else if (p->tag == BuiltinFn(EQUAL)) {
		c = " == ";
	}
	else if (p->tag == BuiltinFn(UNEQUAL)) {
		c = " != ";
	}
	else if (p->tag == BuiltinFn(GREATER_EQUAL)) {
		c = " >= ";
	}
	else if (p->tag == BuiltinFn(LESS_EQUAL)) {
		c = " <= ";
	}
	else if (p->tag == BuiltinFn(GREATER)) {
		c = " > ";
	}
	else if (p->tag == BuiltinFn(LESS)) {
		c = " < ";
	}
	else if (p->tag == BuiltinFn(RULE)) {
		c = " -> ";
	}
	else if (p->tag == BuiltinFn(RULE_DELAYED)) {
		c = " :> ";
	}
	else if (p->tag == BuiltinFn(DOT)) {
		c = ".";
	}
	else if (p->tag == BuiltinFn(POWER)) {
		c = "^";
	}
	else if (p->tag == BuiltinFn(SET)) {
		c = " = ";
	}
	else if (p->tag == BuiltinFn(SET_DELAYED)) {
		c = " := ";
	}
	else if (p->tag == BuiltinFn(CONDITION)) {
		c = " /; ";
	}
	else if (p->tag == BuiltinFn(AND)) {
		c = " && ";
	}
	else if (p->tag == BuiltinFn(OR)) {
		c = " || ";
	}
	else if (p->tag == BuiltinFn(NON_COMMUT_TIMES)) {
		c = " ** ";
	}

	SMathFPrintExpr (p->list[0], p, f);

	for (i=1; i<p->list_count; ++i) {
		fprintf (f, "%s", c);
		SMathFPrintExpr (p->list[i], p, f);
	}

	if (prev && (prev->tag->s_prec > p->tag->s_prec))
		fprintf (f, ")");
}

void
SMathFPrintUnary(p, prev, f)
struct s_node *p;
struct s_node *prev;
FILE	*f;
{

	char	*c = "*unknown*";
	Boolean prefix = False;

	if (p->list_count == 0) {
		fprintf (f, "%s", p->tag->s_name);
		return;
	}

	if (p->tag == BuiltinFn(FACTORIAL)) {
		c = "!";
	}
	else if (p->tag == BuiltinFn(FACTORIAL2)) {
		c = "!!";
	}
	else if (p->tag == BuiltinFn(NOT)) {
		c = "!";
		prefix = True;
	}

	if (prefix) {
		fprintf (f, "%s", c);
	}

	if ((prev && (prev->tag->s_prec > p->tag->s_prec)) ||
		(p->list[0]->list_count > 0))
		fprintf (f, "(");

	SMathFPrintExpr (p->list[0], p, f);

	if ((prev && (prev->tag->s_prec > p->tag->s_prec)) ||
		(p->list[0]->list_count > 0))
		fprintf (f, ")");

	if (!prefix) {
		fprintf (f, "%s", c);
	}

}

/* ARGSUSED */
void
SMathFPrintList (p, prev, f)
struct s_node *p, *prev;
FILE	*f;
{
	register list_index_t	i;

	fprintf (f, "{");

	if (p->list_count)
		SMathFPrintExpr (p->list[0], p, f);

	for (i=1; i<p->list_count; ++i) {
		fprintf (f, ", ");
		SMathFPrintExpr (p->list[i], p, f);
	}

	fprintf (f, "}");
}

void
SMathPrintExpr (p, prev)
struct s_node *p, *prev;
{
	SMathFPrintExpr (p, prev, stdout);
}

void
SMathPrintInfix (p, prev)
struct s_node *p, *prev;
{
	SMathFSMathPrintInfix (p, prev, stdout);
}


struct s_node *
SMathCreateAssocNode(fn, left, right)
struct s_symbol *fn;
struct s_node *left, *right;
{
	register struct s_node *p;

	if (left->tag == fn) {
		p = SMathNodeAddParameter (left, right);
	}
	else if (right->tag == fn) {
		p = SMathNodePrepend (right, left);
	}
	else {
		p = SMathCreateNode (fn, left, right, NULL);
	}

	return p;
}

void
SMathFreeNode (node)
struct s_node	*node;
{
#ifdef notdef
	if (node->tag != (struct s_symbol *) NULL && 
		node->list != (struct s_node **) NULL) {
#endif
	if (node->list != (struct s_node **) NULL) {
		free ((char *) node->list);
	}
	free ((char *) node);
}

void
SMathFreeExpr (node)
struct s_node *node;
{
	register list_index_t	i;

	if (node->list_count > node->list_max) {
		fprintf (stderr, "assertion failed: list_count > list_max\n");
		exit (1);
	}

	for (i=0; i<node->list_count; ++i) {
		if (!node->list[i]) {
			fprintf (stderr, "Assertion failed: null expression in list\n");
			exit (1);
		}
		SMathFreeExpr (node->list[i]);
	}

	SMathFreeNode (node);
}

int
SMathNodeFlatten (e, i)
struct s_node *e;
list_index_t	i;
{
	size_t	size;
	list_index_t	j, k;
	struct	s_node *p = e->list[i];
	register short offset;

#ifdef DEBUG
	printf ("before flattening \"");
	SMathPrintExpr(e, (struct s_node *) NULL);
	printf ("\"\n");
#endif

	offset = p->list_count - 1;

	if (offset > 0) {

		size = (e->list_count + p->list_count) *
			sizeof(struct s_node *);

		if ((e->list =
			(struct s_node **) realloc((malloc_t) e->list, size))
			== (struct s_node **) NULL) {
			fprintf (stderr, "%s", errmsg);
			exit (1);
		}

		e->list_max = e->list_count + p->list_count;

		for (j=e->list_count-1; j > i; --j) {
			e->list[j+offset] = e->list[j];
		}
	}

	for (j=i, k=0; k < p->list_count; ++j, ++k) {
		e->list[j] = p->list[k];
	}

	e->list_count += offset;

	SMathFreeNode (p);

#ifdef DEBUG
	printf ("after flattening \"");
	SMathPrintExpr(e, (struct s_node *) NULL);
	printf ("\"\n");
#endif

	return i;
}

struct s_node *
SMathCreatePatternNode (symb, blank, expr)
char	*symb;
struct	s_symbol *blank;
struct	s_node	*expr;
{
	register struct s_node	*e, *e1;

	if (expr) {
		e1 = SMathCreateNodeBySize (blank, 1);
		e1->list_count = 1;
		e1->list[0] = expr;
	}
	else {
		e1 = SMathCreateNodeBySize (blank, 1);
	}

	if (strlen(symb) == 0) {
		e = e1;
	}
	else {
		e = SMathCreateNodeBySize (BuiltinFn(PATTERN), 2);
		e->list_count = 2;
		e->list[0] = SMathCreateSymbolNode (SMathAddSymbol (symb));
		e->list[1] = e1;
	}

	return e;
}
