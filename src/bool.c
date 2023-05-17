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
#include <bool.h>

struct s_node *
_SMathAndRules (e, mflag)
struct s_node *e;
Boolean *mflag;
{
	register list_index_t i;
	register struct s_symbol *s;
	register struct s_node *e1;

	for (i=e->list_count-1; ; --i) {
		e->list[i] = SMathEvaluate(&e->list[i], e, mflag, NoContext);
		s = e->list[i]->tag;
		if (s == BuiltinFn(FALSE_SYM)) {
			*mflag = True;
			SMathFreeExpr (e);
			return SMathFalse();
		}
		else if (s == BuiltinFn(TRUE_SYM)) {
			*mflag = True;
			SMathNodeRemoveParameter (e, i);
		}
		if (i ==0)
			break;
	}

	if (e->list_count == 0) {
		*mflag = True;
		SMathFreeExpr (e);
		return SMathTrue();
	}

	if (e->list_count == 1) {
		*mflag = True;
		e1 = e->list[0];
		SMathFreeNode (e);
		return e1;
	}

	return e;
}

struct s_node *
_SMathOrRules (e, mflag)
struct s_node *e;
Boolean *mflag;
{
	register list_index_t i;
	register struct s_symbol *s;
	register struct s_node *e1;

	for (i=e->list_count-1; ; --i) {
		e->list[i] = SMathEvaluate(&e->list[i], e, mflag, NoContext);
		s = e->list[i]->tag;
		if (s == BuiltinFn(TRUE_SYM)) {
			*mflag = True;
			SMathFreeExpr (e);
			return SMathTrue();
		}
		else if (s == BuiltinFn(FALSE_SYM)) {
			*mflag = True;
			SMathNodeRemoveParameter (e, i);
		}
		if (i == 0)
			break;
	}

	if (e->list_count == 0) {
		*mflag = True;
		SMathFreeExpr (e);
		return SMathFalse();
	}

	if (e->list_count == 1) {
		*mflag = True;
		e1 = e->list[0];
		SMathFreeNode (e);
		return e1;
	}

	return e;
}

struct s_node *
_SMathNotRules (e, mflag)
struct s_node *e;
Boolean *mflag;
{
	register struct s_symbol *s;
	
	if (e->list_count == 0)
		return e;

	if (e->list_count != 1) {
		/* error */
	}

	e->list[0] = SMathEvaluate(&e->list[0], e, mflag, NoContext);
	s = e->list[0]->tag;
	if (s == BuiltinFn(TRUE_SYM)) {
		*mflag = True;
		SMathFreeExpr (e);
		return SMathFalse();
	}
	else if (s == BuiltinFn(FALSE_SYM)) {
		*mflag = True;
		SMathFreeExpr (e);
		return SMathTrue();
	}

	return e;
}

struct s_node *
_SMathConditionRules (e, mflag)
struct s_node *e;
Boolean *mflag;
{
	register struct s_node *e1;

	if (e->list_count != 2) {
		/* error */
		return e;
	}

	*mflag = True;
	e->list[1] = SMathEvaluate(&e->list[1], e, mflag, NoContext);
	if (e->list[1]->tag == BuiltinFn(TRUE_SYM)) {
		e->list[0] = SMathEvaluate(&e->list[0], e, mflag, NoContext);
		e1 = SMathCopyExpr(SMathEvaluate(&e->list[0], e, mflag, NoContext));
	}
	else {
		e1 = SMathCreateSymbolNode (BuiltinFn(NULL_SYM));
	}
	SMathFreeExpr (e);
	return e1;
}
