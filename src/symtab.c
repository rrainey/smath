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
#include <parser.h>
#include <stdio.h>
#include <string.h>

void SMathClearSymbol();

struct s_node *
_SMathDollarContextRules(e, mflag)
struct s_node *e;
Boolean *mflag;
{
	SMathFreeExpr (e);
	return SMathCreateSymbolNode (_s_context);
}

struct s_node *
_SMathDollarContextPathRules(e, mflag)
struct s_node *e;
Boolean *mflag;
{
	list_index_t i;
	struct s_node *e1;

	SMathFreeExpr (e);
	e1 = SMathCreateNodeBySize (BuiltinFn(LIST),
		_s_context_path_count);
	for (i=0; i<_s_context_path_count; ++i) {
		e1->list[i] = SMathCreateSymbolNode (_s_context_path[i]);
	}
	e1->list_count = _s_context_path_count;
	return e1;
}

struct s_symbol *
SMathLookup(name, context, insert_loc)
char	*name;
struct s_symbol *context;
struct s_symbol ***insert_loc;
{
	register int	n;
	register struct	s_symbol *cur, **loc = (struct s_symbol **) NULL;

	cur = context->s_top;
	while (cur) {
		n = strcmp (name, cur->s_name);
		if (n == 0) {
			*insert_loc = loc;
			return cur;
		}
		else if (n < 0) {
			loc = &(cur->left);
			cur = cur->left;
		}
		else {
			loc = &(cur->right);
			cur = cur->right;
		}
	}

	*insert_loc = loc;
	return (struct s_symbol *) NULL;
}

/*
 *  Determine the context of a scoped variable. For example,
 *  System`List would return a pointer to System` and simple name would be
 *  set to "List".
 */

struct s_symbol *
SMathParseContext(name, simple_name, cxt, prev)
char	*name;
char	*simple_name;
struct s_symbol **cxt;
struct s_symbol ***prev;
{
	register char *p = name, *end, save;
	struct s_symbol *context = _s_root_context, *cur, **loc;

	strcpy (simple_name, "");

	while ((end = strchr (p, '`'))) {
		++end;
		if ((save = *end) == '\0') {
			break;
		}
		*end = '\0';
		cur = SMathLookup (p, context, &loc);
		*end = save;
		if (!cur) {
			return (struct s_symbol *) NULL;
		}
		context = cur;
		p = end;
	}

	strcpy (simple_name, p);
	cur = SMathLookup (p, context, &loc);
	*prev = loc;
	*cxt = context;
	return cur;
}

struct s_symbol *
SMathAddSymbol(name)
char	*name;
{
	register int	n, i;
	struct	s_symbol 	*cur, **prev = NULL, *new, *context, *cxt;
	char name1[1024], *p = name1;

	if (strchr (name, '`')) {
	        context = NULL;
		if ((cur = SMathParseContext (name, p, &context, &prev))) {
			return cur;
		}
		if (!context) {
		  fprintf (stderr, "Should never happen: %s does not resolve to a context\n", name);
		  exit (1);
		}
	}
	else {
	    context = _s_context;
	    p = name;

	    for (i=_s_context_path_count; i>=0; --i) {
		cxt = (i == _s_context_path_count) ?
			_s_context :
			_s_context_path[i];

		cur = cxt->s_top;

#ifdef DEBUG
		fprintf (stderr,
		"SMathAddSymbol() looking in %s for \"%s\"\n", cxt->s_name, p);
#endif

		while (cur) {
			n = strcmp (p, cur->s_name);
			if (n == 0) {
#ifdef DEBUG
				fprintf (stderr,
				"SMathAddSymbol() found symbol \"%s\"\n", p);
#endif
				return cur;
			}
			else if (i == _s_context_path_count) {
				if ( n < 0) {
					prev = &(cur->left);
					cur = cur->left;
				}
				else {
					prev = &(cur->right);
					cur = cur->right;
				}
			}
			else if (n < 0)
				cur = cur->left;
			else
				cur = cur->right;
		}
	    }
	}

	new = (struct s_symbol *) malloc (sizeof(struct s_symbol));
	if (!new) {
		fprintf (stderr, "memory allocation error in SMathAddSymbol()\n");
		return new;
	}

	new->s_name = strdup(p);
	new->s_type = TYPE_SYMBOL;
	new->s_refcnt = 1;
	new->s_prec = -50;
	new->s_attr = ATTR_NONE;
	new->s_expr = (struct s_node *) NULL;
	new->s_lhs_proc = (caddr_t (*)()) NULL;
	new->s_print = (void (*)()) NULL;
	new->s_rule_count = 0;
	new->s_rule = (struct s_rule *) NULL;
	new->s_msg_count = 0;
	new->s_msg = (struct s_msg *) NULL;
	new->s_opt_count = 0;
	new->s_opt = (struct s_opt *) NULL;
	new->left = new->right = (struct s_symbol *) NULL;
	new->s_context = context;

#ifdef DEBUG
	fprintf (stderr, "SMathAddSymbol() adding \"%s\"\n", new->s_name);
#endif

	if (context->s_top) {
		if (!prev) {
			fprintf (stderr, "assertion failed: insertion into context impossible\n");
			exit (1);
		}
		*prev = new;
	}
	else
		context->s_top = new;

	return new;
}

void
SMathRemoveSymbol (s, recurse)
struct s_symbol *s;
Boolean recurse;
{
	if (!s)
		return;

	if (s->left != NULL && recurse == True)
		SMathRemoveSymbol (s->left, recurse);

	if (s->right != NULL && recurse == True)
		SMathRemoveSymbol (s->right, recurse);

	SMathClearSymbol (s, False);

	free ((char *) s);
}

struct s_symbol *
SMathAddContext (name, prev)
char *name;
struct s_symbol *prev;
{
	struct s_symbol *new, *old_context;

	old_context = _s_context;
	_s_context = (prev) ? prev : _s_root_context;

	new = SMathAddSymbol (name);
	if (!new) {
	    fprintf (stderr, "fatal memory allocation error\n");
	    exit (1);
	}
	new->s_type = TYPE_CONTEXT;
	new->s_top =  (struct s_symbol *) NULL;

	_s_context = old_context;

	return new;
}

struct s_symbol *
SMathAddBuiltin (name, id, procedure, print, attr)
char	*name;
int	id;
caddr_t	(*procedure)();
void	(*print)();
long	attr;
{
	register struct s_symbol *p;

	p = SMathAddSymbol (name);
	if (!p)
		return p;

	p->s_type = TYPE_BUILTIN;
	p->s_rule_proc = procedure;
	p->s_lhs_proc = (caddr_t (*)()) NULL;
	p->s_print = print;
	p->s_attr = attr;
	if (id != -1)
		_s_builtin[id] = p;
	return p;
}

struct s_symbol *
SMathAddAttribute (name, bit_mask)
char	*name;
long	bit_mask;
{
	register struct s_symbol *p;

	p = SMathAddSymbol (name);
	if (!p)
		return p;

	p->s_type = TYPE_ATTRIBUTE;
	p->s_attr = bit_mask;
	return p;
}

void
SMathClearSymbol (s, recurse)
struct s_symbol *s;
Boolean recurse;
{
	register int i;

	if (recurse) {
		if (s->left)
			SMathClearSymbol (s->left, recurse);
		if (s->right)
			SMathClearSymbol (s->right, recurse);
	}

	for (i=0; i<s->s_rule_count; ++i) {
		SMathFreeExpr (s->s_rule[i].r_expr);
	}
	free ((char *) s->s_rule);
	s->s_rule_count = 0;
}

void
SMathClearContext (c)
struct s_symbol *c;
{
	if (c->s_top) {
		SMathClearSymbol (c->s_top, True);
	}
}
