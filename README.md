
# smath - A Simple Symbolic Mathematics Library and Command Line Tool

This is code I wrote back around 1996 and I'm publishing it here to have it out in the public forum.

It mimics Mathemetica in syntax and operation. I used Mathematica around that time. I even used it to develop
an analytical solution to one of the key simulation requirements in my ACM air combat simulator.

## What can you do with smath?

SMath is more a curiosity than a complete symbolic math processor. Still, it is capable of demonstrating some basic math expression reduction
using simple built-in rules.

```Mathematica
$ ./smath

In[0] := 1 + 1
Out[0] = 2

In[1] := 1 + a
Out[1] = 1 + a

In[2] := a + 1
Out[2] = 1 + a

In[3] := b + b
Out[3] = 2 b

In[4] := Sin[Pi/2]
Out[4] = 1

In[5] := Cos[Pi/2]
Out[5] = 0

In[6] := a / a
Out[6] = 1
```

But beware, many simple expressions will not reduce completely:

```Mathematica
In[6] := (b + c) / (b + c)
Out[6] = (b + c) (b + c)^-1

In[7] := (b+2) (b - 2)
Out[7] = (2 + b) (b - 2)
```

## Building

Building requires a yacc and a lex variant.  byacc, Bison, flex, and lex are supported. On Linux:

```
$ sudo apt get install -y yacc flex
```

There's a simple Makefile, which can be adapted to your local OS.

```
$ cd src
$ make
```

## Reporting bugs

While the GitHub Issue system is enabled, don't expect that I will respond to
error reports.

## MIT License

MIT License

Copyright (c) 1996 Riley Rainey

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.