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
 
#define NO_SMATH_EXTERNS
#include <smath.h>
#include <parser.h>
#include <stdio.h>

extern int _SMathLoaded;
Boolean _SMathGlobalTrace;

extern void SMathFSMathPrintInfix(), SMathFPrintList(), FPrintBlank(), FPrintMessageName();
extern void SMathFPrintUnary();
extern struct s_node *_SMathAddRules(), *_SMathSubtractRules();
extern struct s_node *_SMathTimesRules(), *_SMathDivideRules();
extern struct s_node *_SMathPowerRules();
extern struct s_node *_SMathSetRules(), *_SMathSetDelayedRules();
extern struct s_node *_SMathClearRules();
extern struct s_node *_SMathNotRules();
extern struct s_node *_SMathAndRules(), *_SMathOrRules();
extern struct s_node *_SMathFreeQRules(), *_SMathAtomQRules();
extern struct s_node *_SMathHeadRules(), *_SMathConditionRules();
extern struct s_node *_SMathDumpRules();
extern struct s_node *_SMathGetRules();
extern struct s_node *_SMathMessageRules(), *_SMathMessageNameRules();
extern struct s_node *_SMathMessagesRules();
extern struct s_node *_SMathOnRules(), *_SMathOffRules();
extern struct s_node *_SMathDepthRules();
extern struct s_node *_SMathEqualRules(), *_SMathUnequalRules();
extern struct s_node *_SMathGreaterRules();
extern struct s_node *_SMathLessRules();
extern struct s_node *_SMathQuitRules();
extern struct s_node *_SMathHoldRules();
extern struct s_node *_SMathListRules(), *_SMathRangeRules(), *_SMathPartRules();
extern struct s_node *_SMathDollarContextPathRules(), *_SMathDollarContextRules();
extern struct s_node *_SMathCompundExpressionRules();
extern struct s_node *_SMathContextRules();

/*
 * Left Hand Side Handlers
 */

extern struct s_node *_SMathMessageNameLHS();

/*
 *  Globals
 */

struct s_node	 *_s_result;
struct s_symbol *_s_root_context;
struct s_symbol *_s_cur_context;
struct s_symbol *_s_context;
struct s_symbol **_s_context_path;
int _s_context_path_count = 0;
int _s_block_stack_count = 0;
struct s_symbol *_s_block_stack;

struct s_const	minus_one;
struct s_const	*_s_const_minus_one;

struct builtin_tab {
	char	*s_name;
	int	id;
	struct s_node * (*proc)();
	void	(*print)();
	long	attr;
	};

struct attribute_tab {
	char	*s_name;
	long	bit_mask;
	};

static struct builtin_tab builtin_table[] = {
{ "Plus",	PLUS,  _SMathAddRules, SMathFSMathPrintInfix, ATTR_ORDERLESS  | ATTR_FLAT },
{ "Subtract",	SUBTRACT,  _SMathSubtractRules, SMathFSMathPrintInfix, ATTR_NONE },
{ "Times",	TIMES,	 _SMathTimesRules, SMathFSMathPrintInfix, ATTR_ORDERLESS  | ATTR_FLAT },
{ "NonCommutativeTimes", NON_COMMUT_TIMES, NULL, SMathFSMathPrintInfix, ATTR_FLAT },
{ "Divide",	DIVIDE,  _SMathDivideRules, SMathFSMathPrintInfix, ATTR_NONE },
{ "Dot",	DOT,		NULL, SMathFSMathPrintInfix, ATTR_NONE },
{ "Power",	POWER,		_SMathPowerRules, SMathFSMathPrintInfix, ATTR_NONE },
{ "Derivative",	DERIVATIVE,	NULL, NULL, ATTR_NONE },
{ "Equal",	EQUAL,  _SMathEqualRules, SMathFSMathPrintInfix, ATTR_FLAT },
{ "Unequal",	UNEQUAL,  _SMathUnequalRules, SMathFSMathPrintInfix, ATTR_FLAT },
{ "Greater",	GREATER,	_SMathGreaterRules, SMathFSMathPrintInfix, ATTR_FLAT },
{ "GreaterEqual", GREATER_EQUAL, NULL, SMathFSMathPrintInfix, ATTR_FLAT },
{ "Less",	LESS,		_SMathLessRules, SMathFSMathPrintInfix, ATTR_FLAT },
{ "LessEqual",	LESS_EQUAL,	NULL, SMathFSMathPrintInfix, ATTR_FLAT },
{ "Not",	NOT,  _SMathNotRules, SMathFPrintUnary, ATTR_HOLD_ALL },
{ "And",	AND,  _SMathAndRules, SMathFSMathPrintInfix, ATTR_HOLD_ALL },
{ "Or",		OR,  _SMathOrRules, SMathFSMathPrintInfix, ATTR_HOLD_ALL },
{ "List",	LIST,		NULL, SMathFPrintList, ATTR_NONE },
{ "Factorial",	FACTORIAL,	NULL, SMathFPrintUnary, ATTR_NONE },
{ "Set",	SET,  _SMathSetRules, SMathFSMathPrintInfix, ATTR_HOLD_FIRST },
{ "SetDelayed", SET_DELAYED,  _SMathSetDelayedRules, SMathFSMathPrintInfix, ATTR_HOLD_ALL },
{ "Blank",	BLANK, NULL, FPrintBlank, ATTR_NONE },
{ "BlankSequence", BLANK_SEQUENCE, NULL, FPrintBlank, ATTR_NONE },
{ "BlankNullSequence", BLANK_NULL_SEQUENCE, NULL, FPrintBlank, ATTR_NONE },
{ "Pattern",	PATTERN, NULL, FPrintBlank, ATTR_NONE },
{ "Default",	DEFAULT, NULL, NULL, ATTR_NONE },
{ "Optional",	OPTIONAL, NULL, FPrintBlank, ATTR_NONE },
{ "TagSet",	TAG_SET, NULL, NULL, ATTR_HOLD_ALL },
{ "TagSetDelayed", TAG_SET_DELAYED, NULL, NULL, ATTR_HOLD_ALL },
{ "Condition",	CONDITION,  _SMathConditionRules, SMathFSMathPrintInfix, ATTR_HOLD_ALL },
{ "Attributes", -1,	NULL, NULL, ATTR_NONE },
{ "SetAttributes", -1, NULL, NULL, ATTR_NONE },
{ "ClearAttributes", -1, NULL, NULL, ATTR_HOLD_FIRST },
{ "Clear",	-1,  _SMathClearRules, NULL, ATTR_HOLD_ALL },
{ "Null",	NULL_SYM, NULL, FPrintBlank, ATTR_NONE },
{ "FreeQ",	-1,  _SMathFreeQRules, NULL, ATTR_NONE },
{ "AtomQ",	-1,  _SMathAtomQRules, NULL, ATTR_NONE },
{ "Head",	-1,  _SMathHeadRules, NULL, ATTR_HOLD_ALL },
{ "Depth",	-1,  _SMathDepthRules, NULL, ATTR_HOLD_ALL },
{ "MessageName", MESSAGE_NAME,  _SMathMessageNameRules, FPrintMessageName, ATTR_HOLD_ALL },
{ "Message",	-1,  _SMathMessageRules, NULL, ATTR_NONE },
{ "Messages",	-1, _SMathMessagesRules, NULL, ATTR_NONE },
{ "On",		ON,  _SMathOnRules, NULL, ATTR_HOLD_ALL },
{ "Off",	OFF,  _SMathOffRules, NULL, ATTR_HOLD_ALL },
{ "General",	GENERAL, NULL, NULL, ATTR_NONE },
{ "Part",	PART, _SMathPartRules, NULL, ATTR_NONE },
{ "Dump",	-1,  _SMathDumpRules, NULL, ATTR_NONE },
{ "Get",	GET,  _SMathGetRules, NULL, ATTR_NONE },
{ "Range",	-1,	 _SMathRangeRules, SMathFSMathPrintInfix, ATTR_NONE },
{ "Rule",	RULE,	 NULL, SMathFSMathPrintInfix, ATTR_HOLD_FIRST },
{ "RuleDelayed", RULE_DELAYED,  NULL, SMathFSMathPrintInfix, ATTR_HOLD_ALL },
{ "CompoundExpression", COMPOUND_EXPR, _SMathCompundExpressionRules, NULL, ATTR_NONE },
{ "Hold",	HOLD, NULL, NULL, ATTR_HOLD_ALL },
{ "Quit",	-1,  _SMathQuitRules, NULL, ATTR_NONE },
{ "Exit",	-1,  _SMathQuitRules, NULL, ATTR_NONE },
{ "Context",	-1,  _SMathContextRules, NULL, ATTR_NONE },
{ "$Context",	-1,  _SMathDollarContextRules, NULL, ATTR_NONE },
{ "$ContextPath", -1,  _SMathDollarContextPathRules, NULL, ATTR_NONE },
{ "Out",	OUT,  NULL, NULL, ATTR_NONE },
{ NULL, 0, NULL, NULL, ATTR_NONE },
};

struct attribute_tab _SMathAttributeTable[] = {
	{ "Flat",	ATTR_FLAT },
	{ "Listable",	ATTR_LISTABLE },
	{ "OneIdentity", ATTR_ONE_IDENTITY },
	{ "Orderless",	ATTR_ORDERLESS },
	{ "Protected",	ATTR_PROTECTED },
	{ "Constant",	ATTR_CONSTANT },
	{ "Locked",	ATTR_LOCKED },
	{ "HoldFirst",	ATTR_HOLD_FIRST },
	{ "HoldRest",	ATTR_HOLD_REST },
	{ "HoldAll",	ATTR_HOLD_ALL },
	{ "ReadProtected", ATTR_READ_PROTECTED },
	{ 0, 0 }
	};

struct s_symbol *_s_builtin[396];

extern struct s_symbol *SMathAddBuiltin(), *SMathAddAttribute();

int
SMathInitialize()
{
	struct builtin_tab	*p;
	struct attribute_tab	*q;
	struct s_symbol		*s;
	Boolean			mflag;

	if (_SMathLoaded) {
		return 0;
	}

#ifdef LEAKCHECK
	log_allocations (LoggingDisabled);
#endif

	_SMathGlobalTrace = False;

/*
 *  Initialize context information
 */

	_s_root_context = (struct s_symbol *) malloc (sizeof(struct s_symbol));
	if (!_s_root_context) {
	    fprintf (stderr, "Fatal memory allocation error\n");
	    exit (1);
	}
	_s_root_context->s_name = strdup("");
	_s_root_context->s_type = TYPE_CONTEXT;
	_s_root_context->s_top = (struct s_symbol *) NULL;
	_s_root_context->s_context = (struct s_symbol *) NULL;
	_s_root_context->left = _s_root_context->right =
		(struct s_symbol *) NULL;

	_s_context_path =
		(struct s_symbol **) malloc (sizeof(struct s_symbol *) * 2);
	if (!_s_context_path) {
	    fprintf (stderr, "Fatal memory allocation error\n");
	    exit (1);
	}
	_s_context_path[0] = _s_context = SMathAddContext("System`", NULL);
	_s_context_path_count = 1;

	s = SMathAddBuiltin ("True", TRUE_SYM, (caddr_t (*)()) NULL,
		(void (*)()) NULL, (long) ATTR_NONE);
	if (!s) {
	    fprintf (stderr, "Fatal memory allocation error\n");
	    exit (1);
	}
	s->s_type = TYPE_SYMBOL;

	s = SMathAddBuiltin ("False", FALSE_SYM, (caddr_t (*)()) NULL,
		(void (*)()) NULL, (long) ATTR_NONE);
	if (!s) {
	    fprintf (stderr, "Fatal memory allocation error\n");
	    exit (1);
	}

	s->s_type = TYPE_SYMBOL;

	s = SMathAddBuiltin ("Integer", INTEGER_HEAD, (caddr_t (*)()) NULL,
		(void (*)()) NULL, (long) ATTR_NONE);
	if (!s) {
	    fprintf (stderr, "Fatal memory allocation error\n");
	    exit (1);
	}
	s->s_type = TYPE_SYMBOL;

	s = SMathAddBuiltin ("Real", REAL_HEAD, (caddr_t (*)()) NULL,
		(void (*)()) NULL, (long) ATTR_NONE);
	if (!s) {
	    fprintf (stderr, "Fatal memory allocation error\n");
	    exit (1);
	}
	s->s_type = TYPE_SYMBOL;

	s = SMathAddBuiltin ("Complex", COMPLEX_HEAD, (caddr_t (*)()) NULL,
		(void (*)()) NULL, (long) ATTR_NONE);
	if (!s) {
	    fprintf (stderr, "Fatal memory allocation error\n");
	    exit (1);
	}
	s->s_type = TYPE_SYMBOL;

	s = SMathAddBuiltin ("String", STRING_HEAD, (caddr_t (*)()) NULL,
		(void (*)()) NULL, (long) ATTR_NONE);
	if (!s) {
	    fprintf (stderr, "Fatal memory allocation error\n");
	    exit (1);
	}
	s->s_type = TYPE_SYMBOL;

	for (p=builtin_table; p->s_name; p++) {
		s = SMathAddBuiltin (p->s_name, p->id, p->proc, p->print, p->attr);
		if (!s) {
		    fprintf (stderr, "Fatal memory allocation error\n");
		    exit (1);
		}
	}

	for (q=_SMathAttributeTable; q->s_name; q++) {
		SMathAddAttribute (q->s_name, (long) q->bit_mask);
	}

	minus_one.type = CONST_LONG;
	minus_one.u.long_frac_val.num = -1;
	minus_one.u.long_frac_val.denom = 1;
	_s_const_minus_one = &minus_one;

	for (p=builtin_table; p->s_name; p++) {
		SMathAddBuiltin (p->s_name, p->id, p->proc, p->print, p->attr);
	}

	for (q=_SMathAttributeTable; q->s_name; q++) {
		SMathAddAttribute (q->s_name, (long) q->bit_mask);
	}

	minus_one.type = CONST_LONG;
	minus_one.u.long_frac_val.num = -1;
	minus_one.u.long_frac_val.denom = 1;
	_s_const_minus_one = &minus_one;

/*
 *  Store Precedence Information
 */

	BuiltinFn(AND)->s_prec = -11;
	BuiltinFn(OR)->s_prec = -12;
	BuiltinFn(NOT)->s_prec = -10;
	BuiltinFn(PLUS)->s_prec = -3;
	BuiltinFn(SUBTRACT)->s_prec = -3;
	BuiltinFn(TIMES)->s_prec = -2;
	BuiltinFn(DIVIDE)->s_prec = -2;
	BuiltinFn(POWER)->s_prec = -1;
	BuiltinFn(FACTORIAL)->s_prec = 0;
#ifdef notdef
	BuiltinFn(FACTORIAL2)->s_prec = 0;
#endif
	BuiltinFn(CONDITION)->s_prec = -12;
	BuiltinFn(SET)->s_prec = -20;
	BuiltinFn(SET_DELAYED)->s_prec = -20;

	_SMathLoaded = 1;

/*
 *  Store Set Left-Hand-Side procs
 */

	BuiltinFn(MESSAGE_NAME)->s_lhs_proc =
		(caddr_t (*)()) _SMathMessageNameLHS;

#ifdef notdef
	SMathAddBuiltinSymbol ("$RecursionLimit",
		_SMathRecursionLimitLHS, _SMathRecursionLimitValue);
	SMathAddBuiltinSymbol ("$CommandLine");
	SMathAddBuiltinSymbol ("$Display");
	SMathAddBuiltinSymbol ("$DisplayFunction");
$Echo
$Epilog
$IgnoreEOF
$Interrupted
$Line
$Messages
$Output
$Path
$Post
$Pre
$PrePrint
$System
$Urgent
$Version
$$Media

#endif

/*
 *  load initial definitions
 */

	SMathFreeExpr (SMathGet ("sysinit.m", &mflag));

/*
 *  Now switch to the Global` context.
 */

	_s_context = _s_context_path[1] = SMathAddContext ("Global`", NULL);
	_s_context_path_count = 2; 

#ifdef LEAKCHECK
	log_allocations (LoggingEnabled);
#endif

	return 0;
}
