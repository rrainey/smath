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
_SMathEqualRules (e, mflag)
struct s_node *e;
Boolean *mflag;
{
	register long i;
	Boolean	all_match = True;

	if (e->list_count == 0) {
		return e;
	}

	if (e->list_count == 1) {
		*mflag = True;
		SMathFreeExpr (e);
		return SMathTrue();
	}

	for (i=0; i<e->list_count-1; ++i) {
		if (!SMathIsMatch (e->list[i], e->list[i+1], NULL, False,
			SimpleMatch)) {
			all_match = False;	
		}
	}

	if (all_match) {
		*mflag = True;	
		SMathFreeExpr (e);
		return SMathTrue();
	}

	return e;
}

struct s_node *
_SMathUnequalRules (e, mflag)
struct s_node *e;
Boolean *mflag;
{
	register long i;
	Boolean match_found = False;

	if (e->list_count == 0) {
		return e;
	}

	if (e->list_count == 1) {
		*mflag = True;
		SMathFreeExpr (e);
		return SMathTrue();
	}

	for (i=0; i<e->list_count-1; ++i) {
		if (SMathIsMatch (e->list[i], e->list[i+1], NULL, False,
			SimpleMatch)) {
			match_found = True;			
		}
	}

	if (match_found) {
		*mflag = True;	
		SMathFreeExpr (e);
		return SMathFalse();
	}

	return e;
}

struct s_node *
_SMathGreaterRules (e, mflag)
struct s_node *e;
Boolean *mflag;
{
	Boolean value;

	if (e->list_count == 0) {
		return e;
	}

	if (e->list_count == 2) {
		if (IsConstantNode(e->list[0]) && IsConstantNode(e->list[1])) {
			if (SMathCompareConstants (
				(struct s_const *) e->list[0]->list,
				(struct s_const *) e->list[1]->list) == 1) {
				value = True;
			}
			else {
				value = False;
			}
		}
		else {
			return e;
		}
	}
	else {
		return e;
	}

	*mflag = True;	
	SMathFreeExpr (e);
	return SMathReturnBoolean(value);
}

struct s_node *
_SMathLessRules (e, mflag)
struct s_node *e;
Boolean *mflag;
{
	Boolean value;

	if (e->list_count == 0) {
		return e;
	}

	if (e->list_count == 2) {
		if (IsConstantNode(e->list[0]) && IsConstantNode(e->list[1])) {
			if (SMathCompareConstants (
				(struct s_const *) e->list[0]->list,
				(struct s_const *) e->list[1]->list) == -1) {
				value = True;
			}
			else {
				value = False;
			}
		}
		else {
			return e;
		}
	}
	else {
		return e;
	}

	*mflag = True;	
	SMathFreeExpr (e);
	return SMathReturnBoolean(value);
}

