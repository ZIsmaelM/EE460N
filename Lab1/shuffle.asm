	.ORIG x3000
	AND R1, R1, #0
	LDW R1, R1, MASKA
	AND R1, R0, R1		;R1 contains the top 8 bits
	RSHFA R1, R1, #8	;R1 contains the signed 16 bit equivalent of top 8 bit
	AND R2, R2, #0
	LDW R2, R2, MASKB
	AND R2, R0, R2		;R2 contains the bottom 8 bits
	LSHF R2, R2, #8
	RSHFA R2, R2, #8	;R2 contains the signed 16 bit equivalent of bottom 8 bit
	AND R3, R3, #0
	ADD R4, R1, R2		;R4 contains the sum of the two 16-bit equivalents
	STW R4, R3, DEST
MASKA	.FILL xFF00
MASKB	.FILL x00FF
DEST	.FILL x3050
	.END
