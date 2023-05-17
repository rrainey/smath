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

struct s_matrix {
	unsigned short dim_count;
	unsigned short *dim;
	};

Boolean
SMathIsMatrix (e, info)
struct s_node *e;
struct s_matrix *info;
int	*depth;
{
	*depth = *depth + 1;
	if (e->list[0]->tag != BuiltinFn(LIST)) {
		info->dim = (unsigned short *)
			 malloc (sizeof(unsigned short) * *depth);
		info->dim_count = depth;
	}
	else {
		if (_SMathProbeMatrix (e, info, depth) == False) {
			return False;
		}
		*dim[*depth] = e->list[0]->list_count;
	}
	*depth = *depth - 1;
	return True;
}

Boolean
SMathIsMatrix (e, info)
struct s_node *e;
struct s_matrix *info;
{
	int depth = 0;

	_SMathProbeMatrix (e, info, &depth);
}
	

struct s_node *
SMathTimesList (e, mflag)
struct s_node *e;
Boolean *mflag;
{
	struct s_matrix ainfo, binfo;

	if (e->list < 2) {
		return e;
	}

	a = e->list[0];
	b = e->list[1];
	if (SMathIsMatrix (a, &ainfo) && SMathIsMatrix(b, binfo)) {
		if (:wq

		if (ainfo.dim_count != binfo->dim_count) {
			return e;
		}
	}
}
