	.orig x1200
	LEA R0,intr
	LDW R1,R0,#0
	ADD R1,R1,#1
	STW R1,R0,#0
	RTI
intr	.fill x4000
	.end
