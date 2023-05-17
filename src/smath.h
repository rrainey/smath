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
 
#ifndef _smath_h
#define _smath_h

#include <stdlib.h>

#if defined(_DOS) && !defined(MSDOS)
#define MSDOS
#endif

#include <stdio.h>

#ifndef PARAMS
#ifdef _NO_PROTO
#define PARAMS(arglist) ()
#else
#define PARAMS(arglist)	arglist
#endif
#endif

#include <sys/types.h>
#include <string.h>
#if !(defined(FLEX_SCANNER) && defined(__GNUC__))
#include <malloc.h>
typedef void * malloc_t;
#endif

#ifdef MSDOS
#define BISON
typedef void * caddr_t;
typedef void * malloc_t;
#define strdup(s) _strdup(s)
#endif

#ifdef __STDC__
#ifndef __GNUC__
#include <stdlib.h>
#endif
#define USE_STDARG
#else
#define _NO_PROTO
#endif

#ifdef _NO_CONST
#define const
#endif

#define True		1
#define False		0

typedef int	Boolean;

/*
 *  A list_index_t must be signed for the current implementation of
 *  pattern matching to succeed.
 */

typedef long list_index_t;

/*
 *  Definitions for a given function are stored in this structure
 */

struct s_rule {
	struct s_node *r_expr;	/* the expression for this rule */
	};

/*
 *  Messages are stored in this structure
 */

struct s_msg {
	char	*tag;		/* message tag */
	char	*text;		/* message text */
	Boolean	on;		/* True to print, False to supress printing */
	};

/*
 *  Option values are stored in this structure
 */

struct s_opt {
	struct s_symbol *tag;	/* option symbol */
	struct s_node   *value; /* option value */
	Boolean		optimized;
	Boolean		opt_value;
	};

struct s_symbol {
	char	*s_name;	/* printable name */
	short	s_type;		/* symbol type */
	short	s_prec;		/* precedence used for parenthesizing printouts */
	long	s_refcnt;	/* number of nodes using this symbol */
	long	s_attr;		/* attributes */
	union	{
	struct s_node *u_expr;
	struct {
		caddr_t	(*u_rule_proc)();
		caddr_t (*u_lhs_proc)();
		void	(*u_print)();
	}	u_builtin;
	struct {
		struct s_symbol *u_top;	/* top symbol in this context */
	}	u_context;
	}	u;
	list_index_t	s_rule_count;	/* number of transformation rules defined */
	struct	s_rule	*s_rule; /* list of transformation rules */
	list_index_t	s_msg_count;	/* number of defined messages */
	struct	s_msg	*s_msg;	/* list of messages */
	list_index_t	s_opt_count;
	struct	s_opt	*s_opt;
	struct	s_symbol *left, *right, *s_context;
	};

#define s_expr		u.u_expr
#define s_rule_proc	u.u_builtin.u_rule_proc
#define s_lhs_proc	u.u_builtin.u_lhs_proc
#define s_print		u.u_builtin.u_print
#define s_top		u.u_context.u_top

struct s_node {
	struct	s_symbol *tag;		/* NULL for constants */
	list_index_t list_count;	/* number of defined parameters */
	list_index_t list_max;		/* number of slots available in list */
	struct  s_node	**list;		/* pointer to the parameter list */
	};

#define ConstantType(e)		(((struct s_const *) e->list)->type)
#define ConstantVal(e,type)	((struct s_const *) e->list)->u.type

#define IsNonTerminalNode(e)	((e->tag) && ((e)->tag->s_type != TYPE_SYMBOL))
#define IsTerminalNode(e)	(!IsNonTerminalNode(e))
#define IsExprNode(e)		((e->tag) && ((e)->tag->s_type == TYPE_EXPR))
#define IsConstantNode(e)	(!(e->tag))
#define IsNumericConstantNode(e)	(!(e->tag) && ConstantType(e) != CONST_STRING)
#define IsStringNode(e)		(!(e->tag) && ConstantType(e) == CONST_STRING)
#define IsSymbolNode(e)		((e->tag) && ((e)->tag->s_type == TYPE_SYMBOL))

/*
 *  Symbol Types
 */

#define TYPE_BUILTIN	0
#define TYPE_SYMBOL	1
#define TYPE_EXPR	2
#define TYPE_ATTRIBUTE	3
#define TYPE_OPTION	4
#define TYPE_BUILTIN_SYMBOL	5
#define TYPE_CONTEXT	6


/*
 *  Types of constants
 */

typedef enum _const_enum_t {
  CONST_LONG,      /* A signed whole number (31-bits) */
  CONST_LONG_FRAC, /* fraction (31-bits / 32-bits) */
  CONST_STRING
} const_enum_t;

struct s_const {
	const_enum_t	type;
	union {
	    long	long_val;
	    struct {
		long	num;
		long	denom;
	    } long_frac_val;
	    char	*string_val;
	} u;
	};

struct s_block {
	struct s_symbol *context;
	struct s_symbol *context_path;
	int context_path_count;
	};

struct s_eval_context {
	int	depth;
	int	debug;
	};

#define NoContext ((struct s_eval_context *) 0)

#if !defined(NO_SMATH_EXTERNS)
extern struct s_symbol *_s_root_context;
extern struct s_symbol *_s_context;
extern struct s_symbol *_s_cur_context;
extern struct s_symbol **_s_context_path;
extern int _s_context_path_count;
extern struct s_symbol	*_s_builtin[];
extern struct s_node	*_s_result;
extern struct s_block	*_s_block_stack;
extern int		_s_block_stack_count;
#endif

#define BuiltinFn(n)	_s_builtin[n]

/*
 *  Attribute Bit Mask Definitions
 */

#define ATTR_NONE		0
#define ATTR_FLAT		(1<<0)
#define ATTR_LISTABLE		(1<<1)
#define ATTR_ONE_IDENTITY	(1<<2)
#define ATTR_ORDERLESS		(1<<3)
#define ATTR_PROTECTED		(1<<4)
#define ATTR_CONSTANT		(1<<5)
#define ATTR_LOCKED		(1<<6)
#define ATTR_HOLD_FIRST		(1<<7)
#define ATTR_HOLD_REST		(1<<8)
#define ATTR_HOLD_ALL		(1<<9)
#define ATTR_READ_PROTECTED	(1<<10)

/*
 *  Attribute tests for symbol pointers
 */

#define IsFlat(s)		((s) && (s)->s_attr & ATTR_FLAT)
#define IsListable(s)		((s) && (s)->s_attr & ATTR_LISTABLE)
#define IsOneIdentity(s)	((s) && (s)->s_attr & ATTR_ONE_IDENTITY)
#define IsOrderless(s)		((s) && (s)->s_attr & ATTR_ORDERLESS)
#define IsProtected(s)		((s) && (s)->s_attr & ATTR_PROTECTED)
#define IsConstant(s)		((s) && (s)->s_attr & ATTR_CONSTANT)
#define IsLocked(s)		((s) && (s)->s_attr & ATTR_LOCKED)
#define IsHoldFirst(s)		((s) && (s)->s_attr & ATTR_HOLD_FIRST)
#define IsHoldRest(s)		((s) && (s)->s_attr & ATTR_HOLD_REST)
#define IsHoldAll(s)		((s) && (s)->s_attr & ATTR_HOLD_ALL)

/*
 *  The match_info structure is used to return information about matches
 *  in expressions.
 */

struct match_info {
	list_index_t	match_count;	/* number of list items that matched */
	list_index_t	search_modulus;	/* number of items in tested expr */
	list_index_t	*match;		/* index of each matching list member */
	struct s_node	*new_top;
	};

#define SimpleMatch ((struct match_info **) 0)

extern void SMathFreeExpr PARAMS((struct s_node *));
extern void SMathPrintExpr PARAMS((struct s_node *,
	struct s_node *));
extern void SMathFPrintExpr PARAMS((struct s_node *,
	struct s_node *, FILE *));
extern void SMathFreeNode PARAMS((struct s_node *));
extern void SMathAddRule PARAMS((struct s_symbol *, struct s_node *,
	struct s_symbol *));
extern struct s_node *SMathGet PARAMS((char *, Boolean *));
extern struct s_node *SMathCreateSymbolNode PARAMS((struct s_symbol *));
extern struct s_node *SMathCreateNodeBySize PARAMS((struct s_symbol *,
	list_index_t));
extern struct s_node *SMathCreateIntegerNode PARAMS((const long));
extern struct s_node *SMathCreateConstantNode PARAMS((struct s_const *));
extern struct s_node *SMathCreateNode PARAMS((struct s_symbol *, ...));
extern struct s_node *SMathCreateAssocNode PARAMS((struct s_symbol *,
	struct s_node *, struct s_node *));
extern struct s_node *SMathCreateConstantNode PARAMS((struct s_const *));
extern struct s_node *SMathNodeAddParameter PARAMS((struct s_node *,
	struct s_node *));
extern struct s_node *SMathParse PARAMS((char *));
extern struct s_node *SMathEvaluate PARAMS((struct s_node **,
	struct s_node *,
	Boolean *,
	struct s_eval_context *));
extern struct s_node *SMathCreateStringNode PARAMS((const char *));
extern struct s_node *SMathCopyExpr PARAMS((struct s_node *));
extern struct s_node *SMathCreateIntegerNode PARAMS((const long));
extern struct s_node *SMathNodeRemoveParameters PARAMS((struct s_node *,
	list_index_t *, list_index_t));
extern struct s_node *SMathNodeRemoveParameter PARAMS((struct s_node *,
	list_index_t));
extern struct s_symbol *SMathAddContext PARAMS((char *, struct s_symbol *));
extern struct s_symbol *SMathAddSymbol PARAMS((char *));
extern struct s_symbol *SMathLookup PARAMS((char *, struct s_symbol *,
	struct s_symbol ***));
extern int	SMathCompareConstants PARAMS((const struct s_const *,
	const struct s_const *));
extern Boolean	SMathIsMatch PARAMS((struct s_node *, struct s_node *,
	struct s_node *,
	Boolean, struct match_info **));

#if !defined(MSDOS)
extern int fprintf (FILE *, const char *, ...);
extern int printf (const char *, ...);
#endif

extern Boolean _SMathGlobalTrace;

#ifdef LEAKCHECK
#include <leak.h>
#endif

#endif
