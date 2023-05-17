integrate[y_ + z_, x_] := integrate[y, x] + integrate[z, x]
integrate[a x + b x^2 + 3, x]
integrate[c_ y_, x] := c integrate[y, x] /; FreeQ[c, x]
integrate[a x + b x^2 + 3, x]
integrate[c_, x_] := c x /; FreeQ[c, x]
integrate[a x + b x^2 + 3, x]
integrate[x_^n_.] := x^(n+1)/(n+1) /; FreeQ[n, x] && n != -1
integrate[a x + b x^2 + 3, x]
?integrate
