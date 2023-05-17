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
#include <malloc.h>
#include <stdio.h>

extern struct s_const *StringConstant();

static struct s_node *OnOff();

/*
 *  Search for message s::tag.  Look in the messages defined in symbol first.
 *  If it isn't found there, then look for message General::tag.  If that
 *  exists, then copy General::tag to s::tag and return an index to that.
 *
 *  Returns index of the message in s->s_msg, or -1.
 */

static long
LocateMessage (s, tag)
struct s_symbol *s;
char	*tag;
{
	register struct s_symbol *general = BuiltinFn(GENERAL);
	register struct s_msg *p;
	register long i, j;
	size_t	size;

	for (i=0; i<s->s_msg_count; ++i) {
		if (strcmp(s->s_msg[i].tag, tag) == 0) {
			return i;
		}
	}

	for (i=0; i<general->s_msg_count; ++i) {

		if (strcmp(general->s_msg[i].tag, tag) == 0) {

			j = s->s_msg_count;

			size = ++(s->s_msg_count) * sizeof (struct s_msg);

			if (s->s_msg_count == 1) {
				s->s_msg = (struct s_msg *) malloc (size);
			}
		    else {
				s->s_msg = (struct s_msg *)
					realloc (s->s_msg, size);
			}

			if (!s->s_msg) {
			    fprintf (stderr, "memory allocation failed\n");
			    exit (1);
			}

			p = &s->s_msg[j];

			p->tag = strdup (tag);
			p->text = strdup(general->s_msg[i].text);
			if (p->tag == NULL || p->text == NULL) {
			    fprintf (stderr, "fatal memory allocation error\n");
			    exit (1);
			}
			p->on = general->s_msg[j].on;

			return j;
		}
	}

	return -1;

}

struct s_node *
_SMathMessagesRules (e, mflag)
struct s_node *e;
Boolean *mflag;
{
	register long i;
	register struct s_symbol *s; 
	register struct s_node *a, *e1, *e2;

	if (e->list_count != 1) {
		return e;
	}

	a = e->list[0];
	s = e->list[0]->tag;

	if (s->s_msg_count == 0) {
		*mflag = True;
		e1 = SMathCreateSymbolNode (BuiltinFn(NULL_SYM));
		SMathFreeExpr (e);
		return e1;
	}

	e1 = e;

	for (i=0; i<s->s_msg_count; ++i) {

		e2 =
		    SMathCreateNode(BuiltinFn(RULE),
		    SMathCreateNode (BuiltinFn(MESSAGE_NAME),
			SMathCopyExpr (a),
			SMathCreateConstantNode(StringConstant (s->s_msg[i].tag)),
			NULL),
		    SMathCreateConstantNode(StringConstant (s->s_msg[i].text)),
		    NULL);

		if (i == 1) {
			e1 = SMathCreateNode (BuiltinFn(LIST), e1, e2, NULL);
		}
		else if (i > 1) {
			e1 = SMathNodeAddParameter (e1, e2); 
		}
		else {
			e1 = e2;
		}
		*mflag = True;
	}

	return e1;
}

struct s_node *
_SMathMessageRules (e, mflag)
struct s_node *e;
Boolean *mflag;
{
	register struct s_node *e1;
	register char *p;
	register list_index_t i = 1;

	if (e->list_count == 0) {
		return e;
	}

	if (IsStringNode (e->list[0])) {
		p = ConstantVal(e->list[0], string_val);
		*mflag = True;
		while (*p) {
			if (*p != '`') {
				putc (*p, stdout);
			}
			else if (*(p+1) == '`') {
				++p;
				if (i > e->list_count) {
					fprintf (stdout, "``");
				}
				else if (IsStringNode(e->list[i])) {
				    fprintf (stdout, "%s", 
					ConstantVal(e->list[i], string_val));
				}
				else {
				    SMathFPrintExpr (e->list[i],
					(struct s_node *) NULL, stdout);
				}
				++i;
			}
			else {
				putc (*p, stdout);
			}
			++p;
		}
		putc ('\n', stdout);
		e1 = SMathCreateSymbolNode (BuiltinFn(NULL_SYM));
		SMathFreeExpr (e);
		return e1;
	}
	else if (e->list[0]->tag == BuiltinFn(OFF)) {
		*mflag = True;
		e1 = SMathCreateSymbolNode (BuiltinFn(NULL_SYM));
		SMathFreeExpr (e);
		return e1;
	}

	return e;
}

struct s_node *
_SMathMessageNameLHS (e, mflag)
struct s_node *e;
Boolean *mflag;
{
	register int i;
	size_t   size;
	register struct s_msg *p;
	register struct s_symbol *s = e->list[0]->list[0]->tag;
	char	 *tag;
	register struct s_node *e1;

#ifdef DEBUG
	printf ("MessageName LHS processor entered\n");
#endif

	if (!IsStringNode(e->list[1])) {
		/* error */
		return e;
	}

	if (e->list[0]->list_count != 2) {
		/* error */
		return e;
	}

	tag = ((struct s_const *) e->list[0]->list[1]->list)->u.string_val;

	e1 = SMathCopyExpr (e->list[1]);

	for (i=0; i<s->s_msg_count; ++i) {
		if (strcmp(s->s_msg[i].tag, tag) == 0) {
			SMathFreeExpr (e);	
			goto insert;
		}
	}

	i = s->s_msg_count;

	size = ++(s->s_msg_count) * sizeof (struct s_msg);

	if (s->s_msg_count == 1) {
		s->s_msg = (struct s_msg *) malloc (size);
	}
	else {
		s->s_msg = (struct s_msg *) realloc (s->s_msg, size);
	}

	if (!s->s_msg) {
	    fprintf (stderr, "fatal memory allocation error\n");
            exit (1);
        }

insert:
	p = &s->s_msg[i];

	/*
	 *  Cute.  PREfix 1.1 fails to note the memory leak here
	 *  when the entry in s_msg already exists.  The correct
	 *  procesdure would be to free both text and tag before
	 *  performing the 'goto insert'.
	 */

	p->tag = strdup (tag);
	p->text = strdup(((struct s_const *) e1->list)->u.string_val);
        if (p->tag == NULL || p->text == NULL) {
            fprintf (stderr, "fatal memory allocation error\n");
            exit (1);
        }
	p->on = True;

	*mflag = True;
	return e1;
}

struct s_node *
_SMathMessageNameRules (e, mflag)
struct s_node *e;
Boolean *mflag;
{
	register long j;
	register struct s_symbol *s;
	char	 *tag;
	register struct s_node *e1;

	if (e->list_count == 0 ) {
		/* error */
		return e;
	}

	s = e->list[0]->tag;
	tag = ((struct s_const *) e->list[1]->list)->u.string_val;

	if ((j = LocateMessage (s, tag)) >= 0) {
		if (s->s_msg[j].on) {
			e1 = SMathCreateStringNode (s->s_msg[j].text);
		}
		else {
			e1 = SMathCreateNodeBySize (BuiltinFn(OFF), 1);
			e1->list_count = 1;
			e1->list[0] = SMathCreateStringNode (s->s_msg[j].text);
		}
		goto found;
	}

/* error */

	e1 = SMathCreateSymbolNode (BuiltinFn(NULL_SYM));

found:
	*mflag = True;
	SMathFreeExpr (e);
	return e1;
}

void
FPrintMessageName (p, prev, f)
struct s_node *p;
struct s_node *prev;
FILE	*f;
{
	if (p->list_count < 2 ||
		((struct s_const *) p->list[1]->list)->type != CONST_STRING)
		return;

	SMathFPrintExpr (p->list[0], p, f);
	fprintf (f, "::%s",
		((struct s_const *) p->list[1]->list)->u.string_val);
}

struct s_node *
_SMathOffRules (e, mflag)
struct s_node *e;
Boolean *mflag;
{
	return OnOff (e, mflag, False);
}

struct s_node *
_SMathOnRules (e, mflag)
struct s_node *e;
Boolean *mflag;
{
	return OnOff (e, mflag, True);
}

static struct s_node *
OnOff (e, mflag, flag)
struct s_node *e;
Boolean *mflag;
Boolean flag;
{
	register long i, j;
	register struct s_symbol *s;

	if (e->list_count == 1 && IsConstantNode(e->list[0])) {
		return e;
	}

	if (e->list_count == 0) {
		s = BuiltinFn(GENERAL);
		j = LocateMessage (s, "trace");
		_SMathGlobalTrace = flag;
		if (j >= 0) {
			s->s_msg[j].on = flag;
		}
	}
	else {
	    for (i=0; i<e->list_count; ++i) {

/*
 *  On[Symbol]
 */

			if (e->list[i]->list_count == 0) {
				s = e->list[i]->tag;
				j = LocateMessage (s, "trace");
			}

/*
 *  On[Symbol::message
 */

			else {
				s = e->list[i]->list[0]->tag;
				j = LocateMessage (s,
					((struct s_const *) e->list[i]->list[1]->list)->u.string_val);
			}
			if (j >= 0) {
				s->s_msg[j].on = flag;
			}
	    }
	}


	SMathFreeExpr (e);
	*mflag = True;
	return SMathCreateSymbolNode (BuiltinFn(NULL_SYM));
}
