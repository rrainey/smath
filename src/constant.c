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
#include <limits.h>
#include <malloc.h>
#include <stdio.h>
#include <string.h>

static void CanonicalizeConstant();

struct s_const *
StringConstant(s)
char	*s;
{
	register struct s_const *new;

	new = (struct s_const *) malloc (sizeof(struct s_const));
	if (!new) {
		fprintf (stderr, "fatal memory allocation error\n");
		exit (1);
	}
	new->type = CONST_STRING;
	new->u.string_val = strdup (s);

	return new;
}

struct s_const *
CopyConstant(old)
struct s_const *old;
{
	register struct s_const *new;

	new = (struct s_const *) malloc (sizeof(struct s_const));
	if (!new) {
		fprintf (stderr, "fatal memory allocation error\n");
		exit (1);
	}
	*new = *old;
	if (new->type == CONST_STRING)
		new->u.string_val = strdup (old->u.string_val);

	return new;
}

struct s_const *
ParseConstant (s)
char	*s;
{

	int	fp_mode = 0, error = 0;
	char	c;
	long	num = 0, denom = 1;
	struct	s_const *new = (struct s_const *) NULL;

#ifdef DEBUG
	fprintf (stderr, "ParseConstant(\"%s\");\n", s);
#endif

	while (*s) {

		c = *s++;

		if (c == '.') {
			if (!fp_mode) {
				fp_mode = 1;
				continue;
			}
			else {
				error = 1;
				break;
			}
		}

		if (num > LONG_MAX/10) {
			error = 1;
			break;
		}

		num = num * 10 + c - '0';

		if (fp_mode)
			denom *= 10;

	}

	if (!error) {
		new = (struct s_const *) malloc (sizeof(struct s_const));
		if (!new) {
			fprintf (stderr, "fatal memory allocation error\n");
			exit (1);
		}
		if (fp_mode) {
			new->type = CONST_LONG_FRAC;
			new->u.long_frac_val.num = num;
			new->u.long_frac_val.denom = denom;
		}
		else {
			new->type = CONST_LONG;
			new->u.long_val = num;
		}
		CanonicalizeConstant (new);
	}

	return new;
}

void
FPrintConstant (c, f)
struct s_const *c;
FILE	*f;
{

	if (c->type == CONST_LONG)
		fprintf (f, "%ld", c->u.long_val);
	else if (c->type == CONST_LONG_FRAC)
		fprintf (f, "(%ld / %ld)", c->u.long_frac_val.num,
			c->u.long_frac_val.denom);
	else
		fprintf (f, "\"%s\"", c->u.string_val);
}

static long
GCF (a, b)
long	a, b;
{
	long	temp;

#ifdef notdef
	a = labs (a);
	b = labs (b);
#endif
	a = a < 0 ? -a : a;
	b = b < 0 ? -b : b;

#ifdef DEBUG
	fprintf (stderr, "GCF (%ld, %ld) = ", a, b);
#endif

	while (b > 0) {
		temp = a % b;
		a = b;
		b = temp;
	}

#ifdef DEBUG
	fprintf (stderr, "%ld)\n", a);
#endif

	return a;
}

static void
CanonicalizeConstant(c)
struct s_const *c;
{
	register long factor;

	if (c->type == CONST_LONG_FRAC) {
		factor = GCF (c->u.long_frac_val.num,
			c->u.long_frac_val.denom);
		if (factor != 1) {
			c->u.long_frac_val.num /= factor;
			c->u.long_frac_val.denom /= factor;
		}
		if (c->u.long_frac_val.denom == 1) {
			c->type = CONST_LONG;
			c->u.long_val = c->u.long_frac_val.num;
		}
	}
	else if (c->type != CONST_LONG) {
		fprintf (stderr, "assertion failed: attempting to normalize constant of invalid type\n");
		exit (1);
	}
}

void
AddConstant (a, b, r)
struct s_const *a, *b, *r;
{

	register long	factor, m1, m2;

	memset (&r->u, 0, sizeof(&r->u));

	switch (a->type) {

	case CONST_LONG:

		switch (b->type) {

		case CONST_LONG:
			r->type = CONST_LONG;
			r->u.long_val = a->u.long_val + b->u.long_val;
			break;

		case CONST_LONG_FRAC:
			r->type = CONST_LONG_FRAC;
			r->u.long_frac_val.num = a->u.long_val *
				b->u.long_frac_val.denom +
				b->u.long_frac_val.num;
			r->u.long_frac_val.denom = b->u.long_frac_val.denom;
			CanonicalizeConstant (r);
			break;

		default:
			fprintf (stderr, "invalid constant type: %d\n", b->type);
			exit (1);
		}
		break;

	case CONST_LONG_FRAC:

		switch (b->type) {

		case CONST_LONG:
			r->type = CONST_LONG_FRAC;
			r->u.long_frac_val.num = b->u.long_val *
				a->u.long_frac_val.denom +
				a->u.long_frac_val.num;
			r->u.long_frac_val.denom = a->u.long_frac_val.denom;
			CanonicalizeConstant (r);
			break;

		case CONST_LONG_FRAC:
			r->type = CONST_LONG_FRAC;
			factor = GCF (a->u.long_frac_val.denom,
					b->u.long_frac_val.denom);
			m1 = a->u.long_frac_val.denom / factor;
			m2 = b->u.long_frac_val.denom / factor;
			r->u.long_frac_val.num = a->u.long_frac_val.num * m2 +
				b->u.long_frac_val.num * m1;
			r->u.long_frac_val.denom =
				a->u.long_frac_val.denom * m2;
			CanonicalizeConstant (r);
			break;

		default:
			fprintf (stderr, "invalid constant type: %d\n", b->type);
			exit (1);
		}
		break;

	default:
		fprintf (stderr, "invalid constant type: %d\n", a->type);
		exit (1);
	}
}

void
NegateConstant(c)
struct s_const *c;
{

	switch (c->type) {

	case CONST_LONG:
		c->u.long_val = - c->u.long_val;
		break;

	case CONST_LONG_FRAC:
		c->u.long_frac_val.num = - c->u.long_frac_val.num;
		break;

	case CONST_STRING:
		fprintf (stderr, "assertion failed: can't negate string constant\n");
		exit (1);
		break;

	default:
		fprintf (stderr, "assertion failed: invalid constant type\n");
		exit (1);
		break;
	}
}

void
TimesConstant (a, b, r)
struct s_const *a, *b, *r;
{

	memset (&r->u, 0, sizeof(&r->u));

	switch (a->type) {

	case CONST_LONG:

		switch (b->type) {

		case CONST_LONG:
			r->type = CONST_LONG;
			r->u.long_val = a->u.long_val *
				b->u.long_val;
			break;

		case CONST_LONG_FRAC:
			r->type = CONST_LONG_FRAC;
			r->u.long_frac_val.num = a->u.long_val *
				b->u.long_frac_val.num;
			r->u.long_frac_val.denom =
				b->u.long_frac_val.denom;
			CanonicalizeConstant (r);
			break;

		default:
			fprintf (stderr, "invalid constant type: %d\n", b->type);
			exit (1);
		}
		break;

	case CONST_LONG_FRAC:

		switch (b->type) {

		case CONST_LONG:
			r->type = CONST_LONG_FRAC;
			r->u.long_frac_val.num = b->u.long_val *
				a->u.long_frac_val.num;
			r->u.long_frac_val.denom =
				a->u.long_frac_val.denom;
			CanonicalizeConstant (r);
			break;

		case CONST_LONG_FRAC:
			r->type = CONST_LONG_FRAC;
			r->u.long_frac_val.num =
				a->u.long_frac_val.num *
				b->u.long_frac_val.num;
			r->u.long_frac_val.denom =
				a->u.long_frac_val.denom *
				b->u.long_frac_val.denom;
			CanonicalizeConstant (r);
			break;

		default:
			fprintf (stderr, "invalid constant type: %d\n", b->type);
			exit (1);
		}
		break;

	default:
		fprintf (stderr, "invalid constant type: %d\n", a->type);
		exit (1);
	}
}

void
DivideConstant (a, b, r)
struct s_const *a, *b, *r;
{

	memset (&r->u, 0, sizeof(&r->u));

	switch (a->type) {

	case CONST_LONG:

		switch (b->type) {

		case CONST_LONG:
			r->type = CONST_LONG_FRAC;
			r->u.long_frac_val.num = a->u.long_val;
			r->u.long_frac_val.denom = b->u.long_val;
			CanonicalizeConstant (r);
			break;

		case CONST_LONG_FRAC:
			r->type = CONST_LONG_FRAC;
			r->u.long_frac_val.num = a->u.long_val *
				b->u.long_frac_val.denom;
			r->u.long_frac_val.denom = b->u.long_frac_val.num;
			CanonicalizeConstant (r);
			break;

		default:
			fprintf (stderr, "invalid constant type: %d\n", b->type);
			exit (1);
		}
		break;

	case CONST_LONG_FRAC:

		switch (b->type) {

		case CONST_LONG:
			r->type = CONST_LONG_FRAC;
			r->u.long_frac_val.num = a->u.long_frac_val.num;
			r->u.long_frac_val.denom = a->u.long_frac_val.denom *
				b->u.long_val;
			CanonicalizeConstant (r);
			break;

		case CONST_LONG_FRAC:
			r->type = CONST_LONG_FRAC;
			r->u.long_frac_val.num = a->u.long_frac_val.num *
				b->u.long_frac_val.denom;
			r->u.long_frac_val.denom = a->u.long_frac_val.denom *
				b->u.long_frac_val.num;
			CanonicalizeConstant (r);
			break;

		default:
			fprintf (stderr, "invalid constant type: %d\n", b->type);
			exit (1);
		}
		break;

	default:
		fprintf (stderr, "invalid constant type: %d\n", a->type);
		exit (1);
	}
}

int
SMathCompareConstants (a, b)
const struct s_const *a;
const struct s_const *b;
{
#define less -1
#define greater 1
#define equal 0

	register int i = equal;
	register long n, m;

	switch (a->type) {

	case CONST_LONG:
		switch (b->type) {

		case CONST_LONG:
			if (a->u.long_val < b->u.long_val)
				i = less;
			else if (a->u.long_val > b->u.long_val)
				i = greater;
			else
				i = equal;
			break;

		case CONST_LONG_FRAC:
			n = a->u.long_val * b->u.long_frac_val.denom;
			if (n < b->u.long_frac_val.num)
				i = less;
			else if (n > b->u.long_frac_val.num)
				i = greater;
			else
				i = equal;
			break;

		default:
			fprintf (stderr, "invalid constant type: %d\n", b->type);
			exit (1);
		}
		break;

	case CONST_LONG_FRAC:
		switch (b->type) {

		case CONST_LONG:
			n = b->u.long_val * a->u.long_frac_val.denom;
			if (n < a->u.long_frac_val.num)
				i = greater;
			else if (n > a->u.long_frac_val.num)
				i = less;
			else
				i = equal;
			break;

		case CONST_LONG_FRAC:
			n = a->u.long_val * b->u.long_frac_val.denom;
			m = b->u.long_val * a->u.long_frac_val.denom;
			if (n < m)
				i = less;
			else if (n > m)
				i = greater;
			else
				i = equal;
			break;

		default:
			fprintf (stderr, "invalid constant type: %d\n", b->type);
			exit (1);
		}
		break;

	default:
		fprintf (stderr, "invalid constant type: %d\n", a->type);
		exit (1);

	}
	return i;
}
