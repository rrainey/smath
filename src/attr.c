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

extern struct attribute_tab _SMathAttributeTable;
static struct s_node *AttrBitsToList (s);

struct s_node *
_SMathAttributesRules (e, mflag)
struct s_node	*e;
Boolean *mflag;
{

	if (e->list_count > 2 || !IsSymbol(e->list[0]) {
		return e;
	}

/*
 *  Return attribute list
 */

	else if (e->list_count == 1) {
		list = AttrBitsToList (e->list[0]->tag);
		SMathFreeExpr (e);
		*mflag = True;
		return list;
	}
}

static struct s_node *
AttrBitsToList (s)
struct symbol *s;
{
	register int count;
	register struct attribute_tab *p;

	for (count=0, p=_SMathAttributeTable; p->name; ++p) {
	}


}
