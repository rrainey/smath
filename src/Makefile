#LEAK = ../leak/leak.o
LEAK =

OBJS1 = \
	parser.o \
	main.o \
	eval.o \
	pattern.o \
	smath.o \
	node.o \
	symtab.o \
	print.o \
	constant.o \
	sinit.o \
	file.o \
	q.o \
	bool.o \
	relop.o \
	message.o \
	brules.o

.SUFFIXES:
.SUFFIXES:	.c .y .o .l

      LIBS1 = $(LEAK) -ll -lm
   PROGRAMS = smath
     YFLAGS = -dv
CLEAN_THESE = parser.c smath.c y.output parser.output *.cil Auto *.mod *~
       YACC = yacc
        LEX = lex
#CLEAKFLAGS = -DLEAKCHECK -I../leak
   INCLUDES = -I.

#    CFLAGS = -O -g $(INCLUDES) -Wall $(CLEAKFLAGS) -DBISON
#        CC = gcc
     CFLAGS = -g $(INCLUDES) $(CLEAKFLAGS)
         CC = cc
#     PREFIX = prefix
      PREFIX = echo
PREFIXFLAGS = -prefix_opt perform_compile=false \
			-prefix_opt model_search_path=$(HOME)/prefix_1.1_repairs \
			$(INCLUDES)

		#	-prefix_opt report=80003 \

		 SH = /bin/sh

all:	$(PROGRAMS)

smath:	$(OBJS1) $(START_FILES)
	$(CC) $(LDFLAGS) -o $@ $(OBJS1) $(LIBS1) $(LIBES) $(LIBGCC) $(LIBES)

bool.o:		parser.h
brules.o:	parser.h
eval.o:		parser.h
file.o:		parser.h
main.o:		parser.h
message.o:	parser.h
node.o:		parser.h
pattern.o:	parser.h
print.o:	parser.h
q.o:		parser.h
relop.o:	parser.h
sinit.o:	parser.h
smath.o:	parser.h
symtab.o:	parser.h

parser.c parser.h: parser.y
	rm -f parser.c
	$(YACC) $(YFLAGS) parser.y
	-mv y.tab.c parser.c
	-mv parser.tab.c parser.c
	rm -f parser.h
	-mv y.tab.h parser.h
	-mv parser.tab.h parser.h

clean:
	-rm -rf core $(OBJS1) $(PROGRAMS) $(CLEAN_THESE) parser.h parser.c TAGS *.log

lint:
	$(LINT) $(LINTFLAGS) $(SRCS1)

prefix: parser.h parser.c $(MODELS1) 

# Run three passes -- no compilation and no invocation of a link step
# Prefix messages (and some 'make' output) are sent to separate log
# files for each pass.

prefixlogs: parser.h parser.c
	rm -rf prefix?.log
	time $(MAKE) prefix > prefix1.log 2>&1
	rm -f *.mod		# Note -- does not affect Auto/*.mod files
	time $(MAKE) prefix > prefix2.log 2>&1
	rm -f *.mod		# Note -- does not affect Auto/*.mod files
	time $(MAKE) prefix > prefix3.log 2>&1
	rm -f *.mod		# Note -- does not affect Auto/*.mod files
