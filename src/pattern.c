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

struct s_node *SMathSubstitute();
void SMathFreeMatchInfo();

static Boolean
NextValidPermutation (p)
struct match_info *p;
{
	list_index_t i, j;

#ifdef DEBUG
	printf ("NextValidPermutation()\nmatch_count = %d,  search_mod = %d, [",
		p->match_count, p->search_modulus);
	for (i=0; i<p->match_count; ++i) {
		printf ("%ld  ", (long) p->match[i]);
	}
	printf ("]\n");
#endif

/*
 *  Select the next valid permutation
 */

rehunt:
	for (i=p->match_count-1; i>=0; --i) {
		if (++p->match[i] < p->search_modulus) {
			break;
		}
		else {
			p->match[i] = 0;
		}
	}

/*
 *  Are there no more valid permutations?
 */

	if (i < 0) {
		return False;
	}

/*
 *  Validate this permutation.  If there are any duplicate indices, then
 *  reject this and select the next possible permutation.
 */

	for (i=0; i<p->match_count-1; ++i) {
	    for (j=i+1; j<p->match_count; ++j) {
		if (p->match[i] == p->match[j]) {
#ifdef DEBUG
			printf ("rejecting combination : [");
			for (i=0; i<p->match_count; ++i) {
				printf ("%ld  ", p->match[i]);
			}
			printf ("]\n");
#endif
			goto rehunt;
		}
	    }
	}

#ifdef DEBUG
	printf ("next valid combination : [");
	for (i=0; i<p->match_count; ++i) {
		printf ("%ld  ", p->match[i]);
	}
	printf ("]\n");
#endif

	return True;
}

/*
 *  Add a transformation rule to the given symbol
 */

/* ARGSUSED */
void
SMathAddRule (s, e, context)
struct s_symbol *s;
struct s_node *e;
struct s_symbol *context;
{
	register list_index_t i;
	size_t size;
	register struct s_rule *p;
	register struct s_node *e1;

	if (!e) {
	    fprintf (stderr, "assertion failed: Adding NULL rule\n");
	    exit (1);
	}

	for (i=0; i<s->s_rule_count; ++i) {
		e1 = s->s_rule[i].r_expr;
		if (SMathIsMatch(e1->list[0], e->list[0],
			(struct s_node *) NULL, False, SimpleMatch)) {
			SMathFreeExpr (e1);	
			goto insert;
		}
	}

	i = s->s_rule_count;

	size = ++(s->s_rule_count) * sizeof (struct s_rule);

	if (s->s_rule_count == 1) {
		s->s_rule = (struct s_rule *) malloc (size);
	}
	else {
		s->s_rule = (struct s_rule *) realloc ((malloc_t) s->s_rule,
			size);
	}

	if (!s->s_rule) {
	    fprintf (stderr, "fatal memory allocation error\n");
		exit (1);
	}

insert:
	p = &s->s_rule[i];

	p->r_expr = e;
}

/*
 *  Compare a pattern to an expression;  if arm_patterns is True, then
 *  Blank[] and Pattern[] are considered wild-cards.
 *
 *  Returns True iff they match.
 */

Boolean
SMathIsMatch (pattern, expr, top, arm_patterns, m)
struct s_node	*pattern;
struct s_node	*expr;
struct s_node	*top;
Boolean		arm_patterns;
struct match_info **m;
{
	register list_index_t i, j;
	register struct s_node *e1;
	Boolean	oflag = False, ostart = False;
	register struct s_const *a, *b;

	if (!pattern) {
		fprintf (stderr, "assertion failed: pattern passed to SMathIsMatch was NULL\n");
		exit (1);
	}

	if (!expr) {
		fprintf (stderr, "assertion failed: expression passed to SMathIsMatch was NULL\n");
		exit (1);
	}

	if (m) {
		*m = (struct match_info *) NULL;
	}

#ifdef DEBUG
	printf ("comparing pattern \"");
	SMathPrintExpr (pattern, (struct s_node *) NULL);
	printf ("\" to expression \"");
	SMathPrintExpr (expr, (struct s_node *) NULL);
	printf ("\"\n");
#endif

	if (IsConstantNode(pattern) && IsConstantNode(expr)) {
		a = (struct s_const *) pattern->list;
		b = (struct s_const *) expr->list;
		return (SMathCompareConstants (a, b) == 0) ? True : False;
	}

/*
 *  If the root tag of the expression has the attribute "flat", then
 *  special processing is required.  Consider the rule "a_ + a_ := 2 a";
 *  We must have a technique to detect that pattern in orderless, flat
 *  expressions (e.g. "a + 3 + a").
 */

	if (IsFlat(expr->tag) && m != SimpleMatch &&
		expr->list_count >= pattern->list_count) {
		register struct match_info *p;
		struct s_node *otop = top;

		p = (struct match_info *) malloc (sizeof(struct match_info));
		if (!p) {
		    fprintf (stderr, "Fatal memory allocation error\n");
		    exit (1);
		}
		p->match = 
			(list_index_t *) malloc (sizeof(list_index_t));
		if (!p->match) {
		    fprintf (stderr, "Fatal memory allocation error\n");
		    exit (1);
		}
		p->match_count = pattern->list_count;
		p->search_modulus = expr->list_count;
		p->new_top = (struct s_node *) NULL;

/*
 *  Set up the initial test case
 */

		for (i=0; i<p->match_count; ++i) {
			p->match[i] = i;
		}

/*
 *  Now look for a match among all the valid permutations of pattern list
 *  items against each expression list item.
 */

		while (1) {
		    top = SMathCopyExpr (otop);
		    for (i=0; i<p->match_count; ++i) {
			if (SMathIsMatch (top->list[0]->list[i],
				expr->list[p->match[i]],
				top,
				arm_patterns,
				SimpleMatch) == True) {

				if (i == p->match_count-1) {
					p->new_top = top;
					*m = p;
					return True;
				}
			}
			else {
				break;
			}
		    }	/* for ... */

#ifdef DEBUG
		    printf ("match failed at test %ld\n", (long) i);
#endif

		    SMathFreeExpr (top);

		    if (NextValidPermutation (p) == False) {
			SMathFreeMatchInfo(p);
			return False;
		    }

		}	/* while (1) ... */

	}

/*
 *  If it's a blank we're interested in, then we have a match.
 */

	if (pattern->tag == BuiltinFn(BLANK) && arm_patterns) {
		return True;
	}

	if (pattern->tag == BuiltinFn(OPTIONAL) && arm_patterns) {
		pattern = pattern->list[0];
#ifdef DEBUG
		printf ("saw Optional[]; descending one pattern level\n");
#endif
	}

/*
 *  If it is a pattern symbol, then substitute all occurrances of the
 *  current expression in all references to the Pattern[] symbol.
 */

	if (pattern->tag == BuiltinFn(PATTERN) && arm_patterns) {
		top = SMathSubstitute (top, pattern->list[0]->tag, expr);
		return True;
	}

/*
 *  Are any of the patterns arguments optional?
 */

	for (i=0; i<pattern->list_count; ++i) {
		if (pattern->list[i]->tag == BuiltinFn(OPTIONAL)) {
			oflag = True;
			if (i == 0) {
				ostart = True;
			}
			break;
		}
	}

/*
 *  Do the heads and argument counts match ?
 */

	if (oflag) {

		if (pattern->list_count > 2 && pattern->tag != expr->tag) {
			return False;
		}

		if (pattern->list_count == 2 && expr->list_count != 2) {

		    j = ostart ? 1 : 0;

#ifdef DEBUG
	printf ("supplied argument comparing \"");
	SMathPrintExpr (pattern->list[j], (struct s_node *) NULL);
	printf ("\" to expression \"");
	SMathPrintExpr (expr, (struct s_node *) NULL);
	printf ("\"\n");
#endif

		    if (ostart) {
				e1 = SMathCreateNodeBySize (BuiltinFn(DEFAULT), 4);
				e1->list_count = 1;
				e1->list[0] = SMathCreateSymbolNode (pattern->tag);
				top = SMathSubstitute (top, 
					pattern->list[0]->list[0]->list[0]->tag, e1);
				SMathFreeExpr (e1);
		    }

		    if (SMathIsMatch (pattern->list[j], expr, top,
				arm_patterns, SimpleMatch) == False) {
				return False;
		    }

		    if (!ostart) {
				e1 = SMathCreateNodeBySize (BuiltinFn(DEFAULT), 4);
				e1->list_count = 1;
				e1->list[0] = SMathCreateSymbolNode (pattern->tag);
				top = SMathSubstitute (top, 
					pattern->list[1]->list[0]->list[0]->tag, e1);
				SMathFreeExpr (e1);
		    }
		    return True;
		}
	}
	else if (pattern->tag != expr->tag ||
		pattern->list_count != expr->list_count) {
		return False;
	}

/*
 *  For now at least, optional arguments must appear at either the start or
 *  the end of a pattern.
 */

	if (oflag && pattern->list_count != expr->list_count) {

	    if (pattern->list_count < expr->list_count) {
		return False;
	    }

	    j = 0;
	    for (i=0; i<pattern->list_count; ++i) {
		if (pattern->list[i]->tag == BuiltinFn(OPTIONAL)) {
			e1 = SMathCreateNodeBySize (BuiltinFn(DEFAULT), 4);
			e1->list_count = 1;
			e1->list[0] = 
			    SMathCreateSymbolNode (pattern->list[i]->list[0]->tag);
			top = SMathSubstitute (top, 
			    pattern->list[i]->list[0]->tag, e1);
			SMathFreeExpr (e1);
		}
		else {
			if (SMathIsMatch (pattern->list[i],
				expr->list[j], top, arm_patterns, SimpleMatch) == False) {
				return False;
			}
			else {
				j++;
			}
		}
	    }
	}

	for (i=0; i<pattern->list_count; ++i) {
		if (SMathIsMatch (pattern->list[i],
			expr->list[i], top, arm_patterns, SimpleMatch) == False) {
			return False;
		}
	}

	return True;
}

/*
 *  Replace all occurances of symbol "s" with the expression "new" in
 *  "e".  Also replace all occurances of "s_" (Pattern[s, ...]) with
 *  "new".
 */

struct s_node *
SMathSubstitute (e, s, new)
struct s_node	*e; 
struct s_symbol	*s;
struct s_node	*new;
{
	register list_index_t i;

#ifdef DEBUG
	printf ("substitute \"");
	SMathPrintExpr (new, (struct s_node *) NULL);
	printf ("\" for %s in \"", s->s_name);
	SMathPrintExpr (e, (struct s_node *) NULL);
	printf ("\"\n");
#endif

	if (new == NULL) {
		return e;
	}

	if (e->tag == BuiltinFn(PATTERN)) {
		if (e->list_count > 0 && e->list[0]->tag == s) {
			SMathFreeExpr (e);
			return SMathCopyExpr (new);
		}
		else return e;
	}

	if (IsSymbolNode(e) && e->tag == s) {
		SMathFreeExpr (e);
		return SMathCopyExpr (new);
	}

	for (i=0; i<e->list_count; ++i) {
		e->list[i] = SMathSubstitute (e->list[i], s, new);
	}

	return e;
}

void
SMathFreeMatchInfo(p)
struct match_info *p;
{
	if (p->new_top) {
		SMathFreeExpr (p->new_top);
	}
	free ((char *) p->match);
	free ((char *) p);
}
