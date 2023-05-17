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
 
#include <stdio.h>
#include <parser.h>
#include <smath.h>

extern int SMathInitialize();

char line[2048];
extern int fflush();

/*
 *  Used to determine when we're executing a preloaded copy of the program.
 */

int _SMathLoaded = 0;

static int w;

/* ARGSUSED */
int
main(argc, argv)
int	argc;
char	*argv[];
{
	long	cmd = 0;
	Boolean	mflag;
	struct s_node *result;
#ifdef YYDEBUG
	extern int yydebug;

	yydebug = 1;
#endif
	w = 8;

	printf ("This is SMath, Version 0.0 %s\n",
		_SMathLoaded ? "(with preloaded data)" : "");

	if (SMathInitialize()) {
		fprintf (stderr, "During initialization\n");
		exit (1);
	}

	w = printf ("\nIn[%ld] := ", cmd);
	fflush (stdout);

	while (fgets(line, sizeof(line), stdin) != NULL) {

		if (!(result = SMathParse(line))) {
			fprintf (stderr, "The parser detected an error\n");
		}
		else {
			result = SMathEvaluate(&_s_result, _s_result,
				&mflag, NoContext);

			if (result->tag != BuiltinFn (NULL_SYM)) {
				printf ("Out[%ld] = ", cmd);
			}
			SMathPrintExpr (result, (struct s_node *) NULL);
			SMathFreeExpr (result);
		}
		w = printf ("\n\nIn[%ld] := ", ++ cmd);
		fflush (stdout);
	}
	printf ("\n");
	exit (0);
	return 0;
}
