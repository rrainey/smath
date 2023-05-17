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

extern int SMathNodeFlatten();
extern void SMathFreeMatchInfo();

int
SMathExprCompareOrder (a, b)
struct s_node *a, *b;
{
	if (IsConstantNode(a)) {
		return (IsConstantNode(b)) ? 0 : -1;
	}
	else if (IsSymbolNode(a)) {
		if (IsConstantNode(b)) {
			return 1;
		}
		else if (IsSymbolNode(b)) {
			return strcmp(a->tag->s_name,
				b->tag->s_name);
		}
		else {
			return -1;
		}
	}
	return (IsConstantNode(b) || IsSymbolNode(b)) ? 1 : 0;
}

struct s_node *
SMathEvaluate (ei, top, return_mflag, ec)
struct s_node **ei;
struct s_node *top;
Boolean	*return_mflag;
struct s_eval_context *ec;
{
	list_index_t	i, n;
	Boolean	modified, mflag = True, done = False, bogus_mflag;
	struct	s_node *e, *tmp, *tmp1;
	struct	s_symbol *s;
	struct match_info *m = (struct match_info *) NULL;
	struct s_eval_context *ec1;
	int		ec_top = 0;

/*
 *  Housekeeping:  set up an evaluation context, if one has not yet
 *  been supplied.
 */

	e = *ei;

	if (ec == (struct s_eval_context *) NULL) {
		ec1 = (struct s_eval_context *)
			malloc (sizeof(struct s_eval_context));
		if (!ec1) {
		    fprintf (stderr, "memory allocation error\n");
		    exit (1);
		}
		ec1->depth = 0;
		ec1->debug = 0;
		ec_top = 1;
	}
	else {
		ec1 = ec;
	}

	if (++(ec1->depth) > 256) {
		printf ("Recursion limit reached\n");
		return e;
	}

#ifdef DEBUG
	printf ("evaluate expr \"");
	SMathPrintExpr (e, (struct s_node *) NULL);
	printf ("\"\n");
#endif

	while (mflag) {

	mflag = False;

/*
 *  Traverse each leaf node
 */

	for (i=0; i<e->list_count; ++i) {

		if (i == 0) {
			if (!(IsHoldFirst(e->tag) || IsHoldAll(e->tag))) {
			    SMathEvaluate (&e->list[i], top, &mflag, ec1);
			}
		}
		else if (!(IsHoldRest(e->tag) || IsHoldAll(e->tag))) {
			SMathEvaluate (&e->list[i], top, &mflag, ec1);
		}

	}

/*
 *  Order the list, if appropriate.
 */

	if (IsOrderless(e->tag)) {
		for (modified=1; modified; ) {
			modified = 0;
			for (i=0; i<e->list_count-1; ++i) {
			    if (SMathExprCompareOrder (e->list[i],
				e->list[i+1]) > 0) {
				modified = 1;
				tmp = e->list[i];
				e->list[i] = e->list[i+1];
				e->list[i+1] = tmp;
				mflag = True;
			    }
			}
		}
	}

/*
 *  Flatten, if appropriate
 */

	if (IsFlat(e->tag)) {
		n = e->list_count;
		for (i=0; i < n; ++i) {
			if (e->tag == e->list[i]->tag) {
				SMathNodeFlatten (e, i);
				n = e->list_count;
				mflag = True;
			}
		}
	}

/*
 *  Invoke applicable transformation rule(s)
 *
 */


	if ((s = e->tag) != NULL) {

#ifdef DEBUG
		printf ("entering rule matching phase for ");
		SMathPrintExpr (e, (struct s_node *) NULL);
		printf ("\n");
		printf ("(%d rules defined for %s)\n",
			s->s_rule_count, s->s_name);
#endif

	    for (i=0; i<s->s_rule_count && !done; ++i) {

		tmp = SMathCopyExpr (s->s_rule[i].r_expr);

#ifdef DEBUG1
	printf ("rule \"");
	SMathPrintExpr (s->s_rule[i].r_expr, (struct s_node *) NULL);
	printf ("\"\n");
	printf ("rule \"");
	SMathPrintExpr (tmp, (struct s_node *) NULL);
	printf ("\"\n");
#endif


/*
 *  Does the given pattern match this expression?
 */

		if (SMathIsMatch (tmp->list[0], e, tmp, True, &m)) {

/*
 *  The pattern matched.  Evaluate the rhs.  If it evaluates to "Null", then
 *  no assignment is made, otherwise replace the current expression with
 *  the result of the evaluated rhs.
 */

			if (m) {
				SMathFreeExpr (tmp);
				tmp = m->new_top;
				m->new_top = (struct s_node *) NULL;
			}
			tmp1 = SMathCopyExpr (tmp->list[1]);
			SMathEvaluate (&tmp1, top, &bogus_mflag, ec1);
			if (tmp1->tag == BuiltinFn (NULL_SYM)) {
				SMathFreeExpr (tmp1);
			}
			else if (m) {
#ifdef DEBUG
				SMathPrintExpr (e, (struct s_node *) NULL);
				printf (" matched\n(parameters ");
				for (i=0; i<m->match_count; ++i) {
					printf ("%d  ", m->match[i]);
				}
				printf (")\n");
#endif
				if (_SMathGlobalTrace) {
				    SMathPrintExpr (top, (struct s_node *) NULL);
				    printf ("   becomes   ");
				}

				*ei = e = SMathNodeRemoveParameters (e,
					m->match, m->match_count);
				*ei = e = SMathNodeAddParameter (e, tmp1);
				done = mflag = True;
				SMathFreeMatchInfo (m);

				if (_SMathGlobalTrace) {
				    SMathPrintExpr (top, (struct s_node *) NULL);
				    printf ("\n");
				}
			}
			else {

#ifdef DEBUG
				printf ("substituting ");
				SMathPrintExpr (tmp1, (struct s_node *) NULL);
				printf (" for ");
				SMathPrintExpr (e, (struct s_node *) NULL);
				printf ("\n");
#endif

				if (_SMathGlobalTrace) {
				    SMathPrintExpr (e, (struct s_node *) NULL);
				    printf (" -> ");
				    SMathPrintExpr (tmp1, (struct s_node *) NULL);
				    printf ("\n");

				    SMathPrintExpr (top, (struct s_node *) NULL);
				    printf ("   becomes   ");
				}
			
				SMathFreeExpr(e);
				*ei = e = tmp1;

				if (_SMathGlobalTrace) {
				    SMathPrintExpr (top, (struct s_node *) NULL);
				    printf ("\n");
				}
				done = mflag = True;
			}
		}

		SMathFreeExpr (tmp);

		if (done) {
			done = False;
			break;
		}

	    }

/*
 *  Invoke the builtin rule procedure, if one exists for this function.
 *  This procedure may rearrange the expression in any way.
 */

	if ((e->tag != NULL) && (e->tag->s_type != TYPE_SYMBOL) &&
		(e->tag->s_rule_proc != NULL)) {
#ifdef DEBUG
		printf ("invoking rule processor %s on \"", e->tag->s_name);
		SMathPrintExpr(e, (struct s_node *) NULL);
		printf ("\"\n");
#endif
		*ei = e = (struct s_node *) (*e->tag->s_rule_proc)(e, &mflag);
	}

	if (mflag)
		*return_mflag = mflag;
	}

#ifdef DEBUG
	if (mflag) {
		printf ("at bottom of eval loop \"");
		SMathPrintExpr (e, (struct s_node *) NULL);
		printf ("\"\n");
	}
#endif

	}  /* while */

	if (ec_top) {
	  free (ec1);
	}

	return e;
}
