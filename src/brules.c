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

extern void NegateConstant();

static struct s_node * _SMathSet ();

extern void AddConstant(), TimesConstant(), DivideConstant();

static long Depth();
extern int unexec(), main();

#if !defined(MSDOS) && !defined(sun)
/*#define CAN_DUMP */
#endif

struct s_node *
_SMathHeadRules(e, mflag)
struct s_node *e;
Boolean *mflag;
{
	register struct s_const *con;
	register struct s_node *e1;
	register struct s_symbol *s = (struct s_symbol *) NULL;

	if (e->list_count == 0)
		return e;

	if (e->list_count != 1) {
		/* error */
		return e;
	}

	*mflag = 1;

	if (IsConstantNode (e->list[0])) {
		con = (struct s_const *) e->list[0]->list;
		switch (con->type) {
		case CONST_STRING:
			s = BuiltinFn (STRING_HEAD);
			break;
		case CONST_LONG:
			s = BuiltinFn (INTEGER_HEAD);
			break;
		case CONST_LONG_FRAC:
			s = BuiltinFn (REAL_HEAD);
			break;
		}
		e1 = SMathCreateSymbolNode(s);
	}
	else {
		e1 = SMathCreateSymbolNode (e->list[0]->tag);
	}

	SMathFreeExpr (e);
	return e1;
}

struct s_node *
_SMathAddRules(e, mflag)
struct s_node *e;
Boolean *mflag;
{
	register list_index_t i, j, n = e->list_count - 1;
	struct s_node  *e1;
	struct s_const r;

	for (i=0; i<n; ++i) {
		if (IsConstantNode(e->list[i]) && 
			IsConstantNode(e->list[i+1])) {
			AddConstant((struct s_const *) e->list[i]->list,
				(struct s_const *) e->list[i+1]->list, &r);
			*((struct s_const *) e->list[i]->list) = r;
			SMathFreeNode (e->list[i+1]);
			for (j=i+1; j<n; ++j) {
				e->list[j] = e->list[j+1];
			}
			--n;
			--i;
			--e->list_count;
			*mflag = True;
		}
	}

	if (e->list_count == 1) {
		e1 = e->list[0];
		SMathFreeNode (e);
		e = e1;
	}

	return e;
}

struct s_node *
_SMathSubtractRules(e, mflag)
struct s_node *e;
Boolean *mflag;
{
	register list_index_t i, j, n = e->list_count - 1;
	struct s_node  *e1;
	struct s_const r;

	for (i=0; i<n; ++i) {
		if (IsNumericConstantNode(e->list[i]) && 
			IsNumericConstantNode(e->list[i+1])) {
			NegateConstant((struct s_const *)
				e->list[i+1]->list);
			AddConstant((struct s_const *) e->list[i]->list,
				(struct s_const *) e->list[i+1]->list, &r);
			*((struct s_const *) e->list[i]->list) = r;
			SMathFreeNode (e->list[i+1]);
			for (j=i+1; j<n; ++j) {
				e->list[j] = e->list[j+1];
			}
			--n;
			--i;
			--e->list_count;
			*mflag = True;
		}
	}

	if (e->list_count == 1) {
		e1 = e->list[0];
		SMathFreeNode (e);
		e = e1;
	}

	return e;
}

struct s_node *
_SMathTimesRules(e, mflag)
struct s_node *e;
Boolean *mflag;
{
	register list_index_t i, j, n = e->list_count - 1;
	struct s_node  *e1;
	struct s_const r;

	for (i=0; i<n; ++i) {
		if (IsNumericConstantNode(e->list[i]) && 
			IsNumericConstantNode(e->list[i+1])) {
			TimesConstant((struct s_const *) e->list[i]->list,
				(struct s_const *) e->list[i+1]->list, &r);
			*((struct s_const *) e->list[i]->list) = r;
			SMathFreeNode (e->list[i+1]);
			for (j=i+1; j<n; ++j) {
				e->list[j] = e->list[j+1];
			}
			--n;
			--i;
			--e->list_count;
			*mflag = True;
		}
	}

	if (e->list_count == 1) {
		e1 = e->list[0];
		SMathFreeNode (e);
		e = e1;
	}

	return e;
}

struct s_node *
_SMathDivideRules(e, mflag)
struct s_node *e;
Boolean *mflag;
{
	register list_index_t i, j, n = e->list_count - 1;
	struct s_node  *e1;
	struct s_const r;

	for (i=0; i<n; ++i) {
#ifdef notdef
		if (IsNumericConstantNode(e->list[i+1]) && 
		((struct s_const *) e->list[i+1]->list)->type == CONST_LONG &&
		((struct s_const *) e->list[i+1]->list)->u.long_val == 1)
		{
			for (j=i+1; j<n; ++j) {
				e->list[j] = e->list[j+1];
			}
			--n;
			--i;
			--e->list_count;
			*mflag = True;
		}
#endif
		if (IsNumericConstantNode(e->list[i]) && 
			IsNumericConstantNode(e->list[i+1])) {
			DivideConstant((struct s_const *) e->list[i]->list,
				(struct s_const *) e->list[i+1]->list, &r);
			*((struct s_const *) e->list[i]->list) = r;
			SMathFreeNode (e->list[i+1]);
			for (j=i+1; j<n; ++j) {
				e->list[j] = e->list[j+1];
			}
			--n;
			--i;
			--e->list_count;
			*mflag = True;
		}
	}

	if (e->list_count == 1) {
		e1 = e->list[0];
		SMathFreeNode (e);
		e = e1;
	}

	return e;
}

struct s_node *
_SMathPowerRules(e, mflag)
struct s_node *e;
Boolean *mflag;
{
	register list_index_t i, n = e->list_count;
	struct s_node  *e1;
	struct s_const r, p;

	for (i=n-2; i>0; --i) {
		if (IsNumericConstantNode(e->list[i]) && 
			IsNumericConstantNode(e->list[i+1])) {
			TimesConstant((struct s_const *) e->list[i]->list,
				(struct s_const *) e->list[i+1]->list, &r);
			*((struct s_const *) e->list[i]->list) = r;
			SMathFreeNode (e->list[i+1]);
			-- e->list_count;
			*mflag = True;
		}
	}

	if (e->list_count == 2 && IsNumericConstantNode(e->list[0]) &&
		IsNumericConstantNode(e->list[1]) &&
		ConstantType(e->list[1]) == CONST_LONG &&
		ConstantVal(e->list[1],long_val) > 0) {
		p = *((struct s_const *) e->list[0]->list);
		for (i=1; i<ConstantVal(e->list[1],long_val); ++i) {
			TimesConstant((struct s_const *) e->list[0]->list, &p, &r);
			*((struct s_const *) e->list[0]->list) = r;
		}
		SMathFreeNode (e->list[1]);
		-- e->list_count;
		*mflag = True;
	}
	else if (e->list_count == 1) {
		e1 = e->list[0];
		SMathFreeNode (e);
		e = e1;
	}

	return e;
}


struct s_node *
_SMathSetRules(e, mflag)
struct s_node *e;
Boolean *mflag;
{
	return _SMathSet (e, True, mflag);
}

struct s_node *
_SMathSetDelayedRules(e, mflag)
struct s_node *e;
Boolean *mflag;
{
	return _SMathSet (e, False, mflag);
}

static struct s_node *
_SMathSet (e, evaluate, mflag)
struct s_node *e;
Boolean evaluate;
Boolean *mflag;
{
	struct s_node	*tmp;
	register struct s_symbol *c = (struct s_symbol *) NULL;

	if (e->list_count != 2) {
		fprintf (stderr, "invalid arg count\n");
		return e;
	}

	if (IsProtected (e->list[0]->tag)) {
		fprintf (stderr, "Assignment attempted\
 on a protected symbol: %s\n", e->list[0]->tag->s_name);

/*  not done:  remove context c & all symbols defined by it */
	}
	else {
		if (evaluate) {
			e->list[1] = SMathEvaluate (&e->list[1], e->list[1],
				mflag, NoContext);
		}

		if (e->list[0]->tag->s_type == TYPE_BUILTIN &&
			e->list[0]->tag->s_lhs_proc) {
#ifdef DEBUG
			printf ("invoking lhs proc for %s with \"",
				e->list[0]->tag->s_name);
			SMathPrintExpr (e, (struct s_node *) NULL);
			printf ("\"\n");
#endif
			e->list[0] = (struct s_node *)
				(*e->list[0]->tag->s_lhs_proc)(e, mflag);
		}
		else {
#ifdef DEBUG
			printf ("adding rule \"");
			SMathPrintExpr (e, (struct s_node *) NULL);
			printf ("\" to symbol %s\n", e->list[0]->tag->s_name);
#endif
			SMathAddRule (e->list[0]->tag, SMathCopyExpr(e), c);
		}
	}

	SMathFreeExpr (e);
	tmp = SMathCreateSymbolNode (BuiltinFn(NULL_SYM));
	return tmp;
}

/* ARGSUSED */
struct s_node *
_SMathClearRules(e, mflag)
struct s_node *e;
Boolean	*mflag;
{
	register list_index_t i;

	for (i=0; i<e->list_count; ++i) {
	}

	return e;
}

/*
 *  Create a executable image on disk based on this process's memory image.
 *
 *  Dump["filename"]
 */

struct s_node *
_SMathDumpRules(e, mflag)
struct s_node *e;
Boolean	*mflag;
{
	extern int _start();

	if (e->list_count != 1) {
		fprintf (stderr, "invalid arg count\n");
		return e;
	}

	if (!IsConstantNode(e->list[0]) ||
		((struct s_const *) e->list[0]->list)->type != CONST_STRING) {
		return e;
	}

#ifdef CAN_DUMP
	if (unexec (((struct s_const *) e->list[0]->list)->u.string_val,
		"./smathraw", 0, 0, _start) != 0) {
		fprintf (stderr, "unexec failed. Unable to Dump[] program.\n");
		return e;
	}
#endif

	*mflag = True;
	SMathFreeExpr (e);
	return SMathCreateSymbolNode (BuiltinFn(NULL_SYM));

}

struct s_node *
_SMathDepthRules(e, mflag)
struct s_node *e;
Boolean *mflag;
{
	register long	depth;

	if (e->list_count != 1) {
		return e;
	}

	depth = Depth(e->list[0]);
	SMathFreeExpr (e);
	*mflag = True;

	return SMathCreateIntegerNode (depth);
}

static long
Depth (e)
struct s_node *e;
{
	register list_index_t	ldepth, mdepth = 0, i;

	if (e->list_count == 0) {
		return 1;
	}

	for (i=0; i<e->list_count; ++i) {
		ldepth = (list_index_t) Depth (e->list[i]);
		if (ldepth > mdepth) {
			mdepth = ldepth;
		}
	}

	return mdepth + 1;
}

struct s_node *
_SMathQuitRules(e, mflag)
struct s_node *e;
Boolean *mflag;
{
	int	val = 0;

	if (e->list_count > 0 && IsConstantNode (e->list[0]) && 
		ConstantType(e->list[0]) == CONST_LONG) {
		val = ConstantVal (e->list[0], long_val);
	}

	exit (val);
	return e;
}

struct s_node *
_SMathHoldRules (e, mflag)
struct s_node *e;
Boolean *mflag;
{
	struct s_node *e1;

	if (e->list_count != 1) {
		return e;
	}

	*mflag = True;
	e1 = e->list[0];

	SMathFreeNode (e);
	return e1;
}

struct s_node *
_SMathRangeRules (e, mflag)
struct s_node *e;
Boolean *mflag;
{
	struct s_node *cur, *imax, *di, *test, *list;
	Boolean local_mflag = False, done = False;

	if (e->list_count == 1) {
		cur = SMathCreateIntegerNode (1L);
		imax = e->list[0];
		di = SMathCreateIntegerNode (1L);
	}
	else if (e->list_count == 2) {
		cur = e->list[0];
		imax = e->list[1];
		di = SMathCreateIntegerNode (1L);
	}
	else if (e->list_count == 3) {
		cur = e->list[0];
		imax = e->list[1];
		di = e->list[2];
	}
	else {
		return e;
	}

	SMathFreeNode (e);

/*
 *  From the description of Range[] it appears that it is a Fortran-style
 *  loop test. In other words, if cur > imax, the result is { cur } rather
 *  than an empty list.
 */
	list = SMathCreateNode (BuiltinFn(LIST), NULL);

	for (;!done;) {
		list = SMathNodeAddParameter (list, SMathCopyExpr(cur));

		cur = SMathCreateNode (BuiltinFn(PLUS),
			SMathCopyExpr(di), cur, NULL);
		cur = SMathEvaluate (&cur, cur, &local_mflag, NoContext);

		test = SMathCreateNode (BuiltinFn(GREATER),
			SMathCopyExpr(cur), SMathCopyExpr(imax), NULL);
		test = SMathEvaluate (&test, test, &local_mflag, NoContext);
		if (test->tag == BuiltinFn(TRUE_SYM)) {
			done = True;
		}

		SMathFreeExpr (test);
	}

	SMathFreeExpr (cur);
	SMathFreeExpr (di);
	SMathFreeExpr (imax);
	*mflag = True;

	return list;
}

struct s_node *
_SMathPartRules (e, mflag)
struct s_node *e;
Boolean *mflag;
{
	return e;
}

struct s_node *
_SMathCompundExpressionRules (e, mflag)
struct s_node *e;
Boolean *mflag;
{
	list_index_t i;
	struct s_node *e1;

	if (e->list_count > 0) {
		e1 = e->list[e->list_count-1];
	}
	else {
		return e;
	}

	for (i=0; i<e->list_count-1; ++i) {
		SMathFreeExpr (e->list[i]);
	}

	SMathFreeNode (e);

	return e1;
}

struct s_node *
_SMathContextRules (e, mflag)
struct s_node *e;
Boolean *mflag;
{
	struct s_node *e1;

	if (e->list_count == 1) {
		e1 = SMathCreateSymbolNode (e->list[0]->tag->s_context);
		SMathFreeExpr (e);
	}
	else {
		e1 = e;
	}

	return e1;
}
