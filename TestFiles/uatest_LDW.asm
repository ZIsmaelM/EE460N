	.orig x3000
	LEA R0,A
	LDW R1,R0,#0
	LDW R0,R1,#0
	AND R2,R2,#0
	ADD R2,R2,#5
	HALT
A	.fill x3007
	.end
