f::overflow = "Factorial too large."
f[x_Integer] := If[x > 10, CompoundExpression[Message[f::overflow],Infinty], x!]
f[20]
Off[f::overflow]
f[20]
On[f::overflow]
f[20]
Messages[f]
