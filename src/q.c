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

Boolean SMathFree();

struct s_node *
SMathReturnBoolean (flag)
Boolean	flag;
{
	return SMathCreateSymbolNode (BuiltinFn((flag ? TRUE_SYM:FALSE_SYM)));
}

#define SMathTrue()	SMathReturnBoolean (True)
#define SMathFalse()	SMathReturnBoolean (False)

struct s_node *
_SMathFreeQRules(e, mflag)
struct s_node *e;
Boolean *mflag;
{
	Boolean	result;

	if (e->list_count != 2) {
		/* error */
		return e;
	}

	result = SMathFree (e->list[0], e->list[1]);
	SMathFreeExpr (e);
	*mflag = True;
	return SMathReturnBoolean (result);
}

Boolean
SMathFree (e, f)
struct s_node *e;
struct s_node *f;
{
	register list_index_t i;

	if (SMathIsMatch (f, e, (struct s_node *) NULL, False, SimpleMatch)) {
		return False;
	}

	if (e->list_count > e->list_max) {
	    fprintf (stderr, "assertion failed: list_count > list_max\n");
	    exit (1);
	}

	for (i=0; i<e->list_count; ++i) {
		if (SMathIsMatch (f, e->list[i], (struct s_node *) NULL, False,
			SimpleMatch)) {
			return False;
		}
	}

	return True;
}

struct s_node *
_SMathAtomQRules(e, mflag)
struct s_node *e;
Boolean *mflag;
{
	struct s_node *e1;

	if ((e) && e->list_count == 1 && e->list[0]->list_count == 0)
		e1 = SMathTrue();
	else
		e1 = SMathFalse();

	if (e) {
	    SMathFreeExpr (e);
	}

	*mflag = True;

	return e1;
}
