	.orig x1200
	ADD R6,R6,#-2
	STW R0,R6,#0
	ADD R6,R6,#-2
	STW R1,R6,#0
	ADD R6,R6,#-2
	STW R2,R6,#0
	ADD R6,R6,#-2
	STW R3,R6,#0
	LEA R0,ptsize
	LDW R0,R0,#0
	LEA R1,ptbr
	LDW R1,R1,#0
	LEA R2,mask
	LDW R2,R2,#0
loop	LDW R3,R1,#0
	AND R3,R2,R3
	STW R3,R1,#0
	ADD R1,R1,#2
	ADD R0,R0,#-1
	BRp loop
	LDW R3,R6,#0
	ADD R6,R6,#2
	LDW R2,R6,#0
	ADD R6,R6,#2
	LDW R1,R6,#0
	ADD R6,R6,#2
	LDW R0,R6,#0
	ADD R6,R6,#2
	RTI
ptsize	.fill #128
ptbr	.fill x1000
mask	.fill xfffe
	.end
