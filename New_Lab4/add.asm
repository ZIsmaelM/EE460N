	.orig x3000
	AND R3,R3,#0
	ADD R3,R3,#15
	ADD R3,R3,#5
	LEA R0,intr
	LDW R0,R0,#0
	AND R1,R1,#0
	ADD R1,R1,#1
	STW R1,R0,#0
	LEA R2,data
	AND R5,R5,#0
loop	LDB R4,R2,#0
	ADD R5,R5,R4
	LDB R4,R2,#1
	ADD R5,R5,R4
	ADD R2,R2,#2
	ADD R3,R3,#-2
	BRp loop
	STW R5,R2,#0
	trap x25
data	.fill x0000
intr	.fill x4000 
	.end
