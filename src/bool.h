extern struct s_node *SMathReturnBoolean PARAMS((Boolean));

#define SMathTrue()	SMathReturnBoolean(True)
#define SMathFalse()	SMathReturnBoolean(False)
