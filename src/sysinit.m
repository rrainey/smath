Default[Power] = 1
Default[Plus] = 0
Default[Times] = 1

1 * a_ 		:= a
a_ a_		:= a ^ 2

0 + a_ 		:= a
a_ + a_		:= 2 a
n_. a_ + a_	:= (n+1) a
n_. a_ + m_. a_ := (n+m) a
n_. a_ - m_. a_ := (n-m) a

0 a_		:= 0
1 a_		:= a

a_ / 1		:= a
0 / a_		:= 0

a_ ^ b_ ^ c_	:= a ^ (b c)
a_ ^ 1		:= a
a_ ^ 0		:= 1
a_ ^ n_. a_ ^m_. := a ^ (n + m)

D[a_, x_]	:= 0 /; FreeQ[a, x]
D[u_ + v_, x_]	:= D[u, x] + D[v, x]
D[u_ v_, x_]	:= u D[v, x] + v D[u, x]
D[u_ / v_, x_]	:= (v D[u, x] - u D[v, x]) / v^2
D[x_, x_]	:= 1
D[a_. x_^n_., y_]:= a * n * x^(n - 1) D[x, y] /; FreeQ[a, y]

Sin[x_]/Cos[x_] := Tan[x]
Sin[0]		:= 0
Sin[Pi/2]	:= 1
Cos[0]		:= 1
Cos[Pi/2]	:= 0

General::trace	= "dummy message"
General::argct = "`` called with `` arguments"
